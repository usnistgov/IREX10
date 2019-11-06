/**
 * This software was developed at the National Institute of Standards and Technology (NIST) by
 * employees of the Federal Government in the course of their official duties. Pursuant to title
 * 17 Section 105 of the United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility  whatsoever for its use by other
 * parties, and makes no guarantees, expressed or implied, about its quality, reliability, or any
 * other characteristic.
 */
#include "irex/irex.h"
#include "irex/structs.h"

#include <iostream>
#include <fstream>
#include <csignal>
#include <sys/types.h>
#include <sys/stat.h>
#include <algorithm>
#include <dirent.h>

const static int numCandidates = 10;

using std::string;
using std::cerr;
using std::cout;
using std::endl;
using Irex::IrisImage;
using Irex::ReturnStatus;
using ReturnCode = ReturnStatus::ReturnCode;
using PixelFormat = IrisImage::PixelFormat;
using Label = Irex::IrisImage::Label;

const static string enrollDir = "./enroll",
                    configDir = "./config";

/**
 * Creates an iris_image from a PPM or PGM file.
 * This function isn't intended to fully support the PPM format, only enough to read the
 * validation images.
 */
IrisImage readImage(const string &path)
{
   IrisImage iris;
   
   // Open file.
   std::ifstream is(path);
   if (!is)
   {
      cerr << "Error opening " << path << endl;
      raise(SIGTERM);
   }

   string magicNumber;

   // Read in magic number.
   is >> magicNumber;
   if (magicNumber != "P5" && magicNumber != "P6")
   {   
      cerr << "Image format unsupported." << endl;
      raise(SIGTERM);
   }   

   // Is the image RGB or grayscale?
   iris.pixelFormat = magicNumber == "P5" ? PixelFormat::Grayscale : PixelFormat::RGB;

   uint16_t maxValue;

   // Read in image dimensions and max pixel value.
   is >> iris.width >> iris.height >> maxValue;

   if (!is)
   { 
      cerr << "Premature end of file while reading header." << endl;
      raise(SIGTERM);
   }   

   // Skip line break.
   is.get();

   // Number of bytes to read.
   const uint32_t numBytes = iris.width * iris.height *
                             (iris.pixelFormat == PixelFormat::Grayscale ? 1 : 3);

   iris.data.resize(numBytes);

   // Read in raw pixel data.
   is.read((char*)iris.data.data(), numBytes);
   if (!is)
   {
      cerr << "Only read " << is.gcount() << " of " << numBytes << " bytes." << endl;
      raise(SIGTERM);
   }
   
   return iris;
}


void createTemplates(const std::shared_ptr<Irex::Interface> implementation,
                     const std::vector<string>& imagePaths,
                     std::vector<Irex::DatabaseEntry>& templates,
                     const Irex::TemplateType type)
{
   // Initialize template creation center.
   Irex::ReturnStatus ret = implementation->initializeTemplateCreation(configDir, type);
   if (ret.code != ReturnCode::Success)
   {
      cerr << "Error returned after call to initializeTemplateCreation()." << endl;
      raise(SIGTERM);
   }

   for (const auto imagePath : imagePaths)
   {
      IrisImage iris = readImage(imagePath);

      if (imagePath == "images/enrollment8.pgm")
      {
         // Provide iris coordinates.
         iris.location.limbusCenterX = 320;
         iris.location.limbusCenterY = 240;
         iris.location.limbusRadius  = 119;
         iris.location.pupilRadius   = 38;
      }

      std::vector<IrisImage> irides(1, iris);

      if (imagePath == "images/search1.pgm")
      {
         // Test two-eye support.
         std::reverse(iris.data.begin(), iris.data.end());
         irides.push_back(iris);

         // Eye labels must always be specified whenever more than one image is provided.
         irides.front().label = Label::LeftIris;
         irides.back().label = Label::RightIris;
      }

      Irex::DatabaseEntry e;
      
      // Create enrollment template from image.
      ret = implementation->createTemplate(irides, e.enrollmentTemplate);

      switch (ret.code)
      {
         case ReturnCode::FormatError:
         case ReturnCode::ConfigDirError:
         case ReturnCode::ParticipantError:
            cerr << "Fatal Error during template creation." << endl;
            raise(SIGTERM);
            
         default:
            e.id = imagePath.substr( imagePath.find_last_of("/\\") + 1 );
            templates.push_back(e);
      }
   }
}

/**
 * Creates the enrollment database, searches against it, and outputs validation results to standard
 * output.
 */
int main(int argc, char** argv)
{
   // NOTE: The actual test driver may perform steps like database creation and template creation
   //       in separate executables.


   struct stat info;

   // Ensure the enrollment directory exists.
   if(stat(enrollDir.c_str(), &info) != 0)
   {
      cerr << "Can't open " << enrollDir << endl;
      raise(SIGTERM);
   }
   else if ((info.st_mode & S_IFDIR) == 0)
   {
      cerr << enrollDir << " is not a directory" << endl;
      raise(SIGTERM);
   }

   std::vector<string> searchImages,
                       enrollmentImages;

   // Get list of search and enroll images.
   DIR* dir;

   if ((dir = opendir ("./images/")) == NULL)
   {
      cerr << "Can't read images directory" << endl;
      return EXIT_FAILURE;
   }

   struct dirent *entry;

   while ((entry = readdir (dir)) != NULL)
   {
      const std::string filename(entry->d_name);

      if (filename.rfind("search", 0) == 0)
         searchImages.push_back("./images/" + filename);
      else if (filename.rfind("enroll", 0) == 0)
         enrollmentImages.push_back("./images/" + filename);
   }

   closedir(dir);

   std::shared_ptr<Irex::Interface> implementation = Irex::Interface::getImplementation();

   std::vector<Irex::DatabaseEntry> enrollmentTemplates;
   
   createTemplates(implementation, enrollmentImages, enrollmentTemplates,
                   Irex::TemplateType::Enrollment);

   // Create the enrollment database.
   ReturnStatus ret = implementation->createDatabase(enrollDir, configDir, enrollmentTemplates);
   if (ret.code != ReturnCode::Success)
   {
      cerr << "Non-zero return value from createDatabase()." << endl;
      raise(SIGTERM);
   }

   std::vector<Irex::DatabaseEntry> searchTemplates;

   // Create the search templates.
   createTemplates(implementation, searchImages, searchTemplates,
                   Irex::TemplateType::Search);

   // Initialize identification session.
   ret = implementation->initializeIdentification(enrollDir, configDir);
   if (ret.code != ReturnCode::Success)
   {
      cerr << "Error returned after call to initializeIdentification()." << endl;
      return EXIT_FAILURE;
   }

   // Iterate over search templates.
   for (const auto& searchTemplate : searchTemplates)
   {
      std::vector<Irex::Candidate> candidates;
      
      // Search template against enrollment database.
      ret = implementation->identify(searchTemplate.enrollmentTemplate, numCandidates, candidates);

      if (ret.code == ReturnCode::FormatError ||
          ret.code == ReturnCode::IdentError  ||
          ret.code == ReturnCode::ParticipantError)
      {  
         cerr << "Fatal Error during searching." << endl;
         return EXIT_FAILURE;
      }

      // Write candidates to standard output.
      for (const auto& c : candidates)
      {
         cout << searchTemplate.id << " " << c.id << " " << c.distance << " "
              << static_cast<int>(ret.code) << endl;
      }

   } // end foreach search

   return EXIT_SUCCESS;
}

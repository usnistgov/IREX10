/**
 * This software was developed at the National Institute of Standards and Technology (NIST) by
 * employees of the Federal Government in the course of their official duties. Pursuant to title
 * 17 Section 105 of the United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility  whatsoever for its use by other
 * parties, and makes no guarantees, expressed or implied, about its quality, reliability, or any
 * other characteristic.
 */

/// Dummy implementation.
#include "stub.h"

#include <fstream>
#include <iostream>
#include <algorithm>
#include <utility>

using std::cerr;
using std::endl;

using ReturnCode = Irex::ReturnStatus::ReturnCode;
using PixelFormat = Irex::IrisImage::PixelFormat;

ReturnStatus Stub::initializeTemplateCreation(const string& configDir,
                                              const Irex::TemplateType type)
{
   return ReturnCode::Success;
}

ReturnStatus Stub::createTemplate(vector<Irex::IrisImage>& irides,
                                  vector<uint8_t>& templateData)
{
   // Only use the first iris image.
   const Irex::IrisImage& iris = irides.front();
  
   // Set template to be the pixel value in the center of the image.
   templateData.resize(1, iris.data[ iris.data.size() / 2 ]);

   return ReturnCode::Success;
}


ReturnStatus Stub::createDatabase(const string& enrollmentDirectory,
                                  const string& configDirectory,
                                  const vector<Irex::DatabaseEntry>& templates)
{
   const std::string dataFile = enrollmentDirectory + "/dataFile";
   
   // If file already exists, do nothing.
   if (std::ifstream(dataFile))
      return ReturnCode::Success;

   std::ofstream os(dataFile);
   if (!os)
   {
      cerr << "Failed to create data file." << endl;
      return ReturnCode::EnrollDirError;
   }

   // Write each template to file.
   for (const auto t : templates)
      os << t.id << endl << t.enrollmentTemplate.front();

   os.close();

   return ReturnCode::Success;
}


ReturnStatus Stub::initializeIdentification(const string& enrollmentDirectory,
                                            const string& configDirectory)
{
   const string dataFile = enrollmentDirectory + "/dataFile";

   std::ifstream is(dataFile);
   if (!is)
   {
      cerr << "Failed to open data file." << endl;
      return ReturnCode::EnrollDirError;
   }

   Irex::DatabaseEntry entry;

   // Load each template from the file.
   while (is >> entry.id)
   {
      is.get(); // skip line break
      
      // Load template.
      entry.enrollmentTemplate.resize(1);
      entry.enrollmentTemplate[0] = is.get();

      _database.push_back(entry);
   }

   is.close();

   return ReturnCode::Success;
}

ReturnStatus Stub::identify(const vector<uint8_t>& searchTemplate,
                            const uint32_t numCandidates,
                            vector<Irex::Candidate>& candidates)
{
   // Compare search template to every enrollment template.
   for (const auto& entry : _database)
   {
      Irex::Candidate candidate;

      candidate.id       = entry.id;
      candidate.distance = searchTemplate[0] + (entry.enrollmentTemplate[0] << 8);

      candidates.push_back(candidate);
   }

   // Sort candidates by distance.
   sort(candidates.begin(), candidates.end(),
        [] (const Irex::Candidate& c1, const Irex::Candidate& c2)
           { return c1.distance < c2.distance; });

   // Trim to requested size.
   candidates.resize(numCandidates);

   return ReturnCode::Success;
}

std::shared_ptr<Irex::Interface> Irex::Interface::getImplementation()
{
    return std::make_shared<Stub>();
}

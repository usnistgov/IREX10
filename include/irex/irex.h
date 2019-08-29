/**
 * This software was developed at the National Institute of Standards and Technology (NIST) by
 * employees of the Federal Government in the course of their official duties. Pursuant to title
 * 17 Section 105 of the United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility  whatsoever for its use by other
 * parties, and makes no guarantees, expressed or implied, about its quality, reliability, or any
 * other characteristic.
 */
#ifndef IREX_1N_H_
#define IREX_1N_H_

#include <string>
#include <vector>
#include <cstdint>
#include <cmath>
#include <memory>

#include "irex/structs.h"

using std::string;

namespace Irex
{
   /// Stores a single candidate (returned by Interface::identify()).
   struct Candidate
   {
      /** A unique identifier assigned to this template ("" indicates the candidate is invalid).
       */
      string id = "";
       
      /** Non-negative measure of dissimilarity (*aka* distance) between the searched
       * template and the candidate.
       */
      double distance = HUGE_VAL;
   };


   /// An enrollment template with associated unique identifier (see Interface::createDatabase()).
   struct DatabaseEntry
   {
      /// The enrollment template.
      std::vector<uint8_t> enrollmentTemplate;

      /// Unique identifier assigned to this template.
      string id;
   };

   
   /// Submitted libraries must override these functions.
   class Interface
   {
      public:
         virtual ~Interface() {};

         /**
          * Initialization function, called once prior to one or more calls to createTemplate().
          *
          * The implementation shall tolerate execution of multiple calls to this function from
          * different processes running on the same machine. If participants wish to use the
          * optional \a configDirectory parameter, they must say so in documentation accompanying
          * their submission. NIST will debug if a call to this function returns a non-zero value,
          * contacting the participant if necessary.
          *
          * \param[in] type About to create enrollment or search templates.
          * \param[in] configDirectory A directory containing read-only configuration files and/or
          *               runtime data files.
          * \return An object of type Irex::ReturnStatus.
          */
         virtual ReturnStatus initializeTemplateCreation(const string& configDirectory,
                                                         const TemplateType type) = 0;

         /**
          * Generates a template from a vector of iris images.
          *
          * The implementation must be able to handle iris images having arbitrary pixel
          * dimensions, although primary NIST tests are expected to only operate over images that
          * are 640x480 or 640x512. If a return value of 100 or greater is returned, NIST will
          * attempt to debug, contacting the participant if necessary.
          *
          * Each IrisImage has an optional *quality* field that participants are encouraged to fill
          * in. As per *ISO/IEC 29794-6: Biometric Sample Quality*, values should be between 0 and
          * 100 with higher values indicating better quality, or 255 indicating an inability to
          * provide a quality score. Implementations are not required to fill in this parameter but
          * it would be appreciated by NIST.
          *
          * When specified, the iris boundary variables provide estimates of the limbus center
          * (*limbusCenterX*, *limbusCenterY*) and pupil (*pupilRadius*) and limbus
          * (*limbusRadius*) radii. The estimates should be accurate to within a few pixels. When
          * left unspecified, the participant is encouraged to fill in these values, as it can
          * assist with debugging, but doing so is optional.
          *
          * Each IrisImage in \p irides represents a single iris image (left or right).
          * Implementations must support the following scenarios:
          * -# \p irides contains a single iris image.
          * -# \p irides contains one left and one right iris.
          * -# \p irides contains several images of the same iris.
          * -# \p irides contains \a N images of the left iris and \a N images of the right iris.
          *
          * For scenarios 2-4 the \p eye parameter for each IrisImage in \p irides will always be
          * set to either 1 (\a left) or 2 (\a right) and never 0 (\a unknown / \a undefined).
          * Scenario 2 represents the most common operational use case and is likely to be the most
          * tested.
          *
          * \param[in] irides The iris images from which to create the template.
          * \param[out] templateData Template generated from the iris samples. The template's
          *                format is proprietary and NIST will not access any part of it other to
          *                pass it to createDatabase() and possibly store it temporarily.
          * \return An object of type Irex::ReturnStatus.
          */
         virtual ReturnStatus createTemplate(std::vector<IrisImage>& irides,
                                             std::vector<uint8_t>& templateData) = 0;

         /**
          * Function to create an enrollment database to search against.
          *
          * Enrollment database creation shall be performed after all enrollment processes are
          * complete. It should populate the contents of the enrollment directory with everything
          * that is necessary to perform searches against it. This function allows post-enrollment
          * book-keeping, normalization, and other statistical processing of the generated
          * templates. It should tolerate being called multiple times, altough subsequent calls
          * should probably not do anything.
          *
          * NIST will debug if a call to this function returns a non-zero value. The NIST test
          * harness will never knowingly enroll the same iris under two different identifiers.
          *
          * \param[in] enrollmentDirectory An absolute path to the top-level directory in which the
          *               enrollment database will reside. The implementation will have read and
          *               write access to this directory.
          * \param[in] configDirectory A directory containing read-only configuration files and/or
          *               runtime data files.
          * \param[in] templates The vector of enrollment templates that should comprise the
          *               enrollment database. The implementation should store these templates in
          *               the enrollment directory.
          * \return An object of type Irex::ReturnStatus.
          */
         virtual ReturnStatus createDatabase(const string& enrollmentDirectory,
                                             const string& configDirectory,
                                             const std::vector<DatabaseEntry>& templates) = 0;

         /**
          * Initialization function, to be called once prior to one or more calls to identify().
          *
          * The function may use this function to read data (e.g. templates) from the enrollment
          * directory and load it into memory. NIST will debug if a call to this function
          * returns a non-zero value, contacting the participant if necessary.
          *
          * \param[in] enrollmentDirectory The top-level directory in which the enrollment data was
          *               placed when finalizeEnrollment() was called.
          * \param[in] configDirectory A directory containing read-only configuration files and/or
                          runtime data files.
          * \return An object of type Irex::ReturnStatus.
          */
         virtual ReturnStatus initializeIdentification(const string& enrollmentDirectory,
                                                       const string& configDirectory) = 0;
  
         /**
          * Searches a template against the enrollment database and returns a list of candidates.
          *
          * NIST will typically set the candidate list length to operationally feasible values
          * (e.g. 20), but may decide to extend it to values that approach the size of the
          * enrollment database.
          *
          * \param[in] searchTemplate A search template generated by a call to createTemplate().
          * \param[in] numCandidates The length of the candidate list array to output.
          * \param[out] candidates An array (of length \a numCandidates) of candidates. Each
          *                candidate shall be populated by the implementation and shall be sorted
          *                in ascending order of distance score (i. e. the most similar entry shall
          *                appear first).
          * \return An object of type Irex::ReturnStatus.
          */
         virtual ReturnStatus identify(const std::vector<uint8_t>& searchTemplate,
                                       const uint32_t numCandidates,
                                       std::vector<Candidate>& candidates) = 0;

         /**
          * Return an instance of the implementation.
          */
      	static std::shared_ptr<Interface> getImplementation();
      
   }; /* end Interface class */

} /* end IREX namespace */

#endif /* _IREX_1N_H_ */

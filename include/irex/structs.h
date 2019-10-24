/**
 * This software was developed at the National Institute of Standards and Technology (NIST) by
 * employees of the Federal Government in the course of their official duties. Pursuant to title
 * 17 Section 105 of the United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility  whatsoever for its use by other
 * parties, and makes no guarantees, expressed or implied, about its quality, reliability, or any
 * other characteristic.
 */
#ifndef IREX_STRUCTS_H_
#define IREX_STRUCTS_H_

#include <string>
#include <vector>
#include <cstdint>
#include <cmath>

using std::string;

namespace Irex
{
   /// A structure contain information about the success or failure of a function call.
   struct ReturnStatus
   {
      /// Return codes for functions specified in this API
      enum class ReturnCode
      {
         Success = 0,          ///< Successful completion
         DetectError,          ///< Unable to detect iris in image
         FormatError,          ///< input data in incorrect format
         IdentError,           ///< search template does not contain matchable data
         EnrollDirError,       ///< operation on enrollment directory failed
         ConfigDirError,       ///< config data missing, unreadable, etc.
         Timeout,              ///< Function call took too long
         ParticipantError      ///< participant-defined error
      };

      ReturnCode code;         ///< Return status code
      string description; ///< Optional description of error

      // inline constructor.
      ReturnStatus(const ReturnCode c = ReturnCode::Success, const string& s = "") :
         code(c), description(s) {};
   };

   
   /// Whether to generate search or enrollment templates.
   enum class TemplateType
   {
      Enrollment = 0,      ///< Enrollment Templates
      Search               ///< Search templates
   };


   /// Holds image data for an iris.
   struct IrisImage
   {
      /// The eye label for an iris image (left, right, or unspecified).
      enum class Label
      {
         Unspecified = 0,  ///< Undefined, unspecified, or unknown
         RightIris,        ///< The subject's right iris
         LeftIris          ///< The subject's left iris
      };

      /// The format of the image raster data.
      enum class PixelFormat
      {
         Grayscale = 0,    ///< A The image is grayscale with 8 bits per pixel
         RGB               ///< The image is RGB with with 24 bits per pixel
      };

      /// Approximate location of the iris in the image
      struct IrisAnnulus
      {
         /// Approximate horizontal center of the limbus in pixels (0=unspecified).
         uint16_t limbusCenterX = 0;
         
         /// Approximate vertical center of the limbus in pixels (0=unspecified).
         uint16_t limbusCenterY = 0;
         
         /// Approximation of pupil radius in pixels (0=unspecified).
         uint16_t pupilRadius = 0;
         
         /// Approximation of limbus radius in pixels (0=unspecified).
         uint16_t limbusRadius = 0;
      };
      
      /// Image width in pixels.
      uint16_t width = 640;

      /// Image height in pixels.
      uint16_t height = 480;

      /// Pointer to image raster data (RGBRGBRGB... for RGB data).
      std::vector<uint8_t> data;

      /// Format of the raster data (Grayscale or RGB).
      PixelFormat pixelFormat;

      /// The eye label (left, right, or unspecified).
      Label label = Label::Unspecified;
       
      /** Wavelength (in nanometers) at which the image was acquired (0=unspecified).
       * If this value is unspecified (as will be the case for the OPS IV dataset),
       * it can be assumed that the image was acquired by a standard iris camera
       * at near infrared wavelengths.
       */
      uint16_t wavelength = 0;

      /** An \a optional quality score that participants are encouranged to fill in.
       * The quality score should predict the comparison performance of templates
       * generated from this iris image. As per *ISO/IEC 29794-6: Biometirc Sample
       * Quality*, values should be between 0 and 100 with higher values indicating
       * better quality, or 255 indicating an inability to calculate a quality score.
       * Again, implementations are not required to fill in this value but it would be
       * appreciated by NIST.
       */
      uint8_t quality = 255;
      
      /** Approximate horizontal center of the limbus in pixels (0=unspecified).
       * Provides an estimate of the limbus center (*limbusCenterX*, *limbusCenterY*) and
       * pupil (*pupilRadius*) and limbus (*limbusRadius*) radii. When provided, the
       * estimates should be accurate to within a few pixels.
       *
       * When left unspecified, the participant is encouraged to fill these values in, as
       * it can assist with debugging, but doing so is optional.
       */
      IrisAnnulus location;
   };

} /* end IREX namespace */

#endif /* IREX_STRUCTS_H_ */

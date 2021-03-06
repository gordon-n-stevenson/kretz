#include "itkVersion.h"
#include "itkKretzImageIO.h"
#include "itkIOCommon.h"
#include "itkArray.h"
#include "itkByteSwapper.h"
#include "vnl/vnl_cross.h"

#include "itkMetaDataObject.h"

#include "itksys/SystemTools.hxx"
#include "itksys/Base64.h"

#include <fstream>
#include <sstream>

namespace itk
{

KretzImageIO::KretzImageIO()
{
  this->SetNumberOfDimensions(3); //needed for getting the 3 coordinates of
  this->SetNumberOfComponents(1);
  m_ByteOrder = LittleEndian;     //default
  m_ComponentType = UCHAR;
  m_PixelType = SCALAR;
}

KretzImageIO::~KretzImageIO()
{
}

bool KretzImageIO::CanReadFile(const char *filename)
{
  std::ifstream file;
  try
  {
    this->OpenFileForReading( file, filename );
  }
  catch( ExceptionObject & )
  {
    return false;
  }

  // sniff for the KRETZ signature 
  {
    file.seekg(0,std::ios_base::beg);
    if(file.fail() || file.eof())
    {
      return false;
    }
    char buf[16];
    file.read(buf,16);
    if(file.fail())
    {
      return false;
    }
    std::string sig(buf);
    if(sig == "KRETZFILE 1.0   ")
    {
      return true;
    }
  }
  return false;
}

void KretzImageIO::Read(void *buffer)
{

  // ensure file can be opened for reading, before doing any more work
  std::ifstream inputFileStream;
  this->OpenFileForReading( inputFileStream, m_FileName );

  // Find last position for tag search
  inputFileStream.seekg(5,std::ios::end);
  int endOfTagSearch = inputFileStream.tellg();

  inputFileStream.seekg(16,std::ios::beg); // skips header

  // let any exceptions propagate
  unsigned short tag_shorts[2];
  unsigned int taglength;
  while((inputFileStream.tellg()<endOfTagSearch-8)&&(inputFileStream.tellg()>-1))
  {
    inputFileStream.read(reinterpret_cast<char *>(&tag_shorts), sizeof(tag_shorts)); 
    inputFileStream.read(reinterpret_cast<char *>(&taglength), sizeof(taglength)); 

    Tag tag(tag_shorts[0],tag_shorts[1]);

    if(tag==ImageTag && !m_isDoppler)
    {

      inputFileStream.read(reinterpret_cast<char *>(buffer), taglength); 

    } 
    else if(tag==ImageTagDoppler && m_isDoppler)
    {

      inputFileStream.read(reinterpret_cast<char *>(buffer), taglength); 

    } 
    else {
      inputFileStream.seekg(taglength,std::ios::cur);
    }

  }

  inputFileStream.close();
}

void KretzImageIO::ReadImageInformation()
{
  std::ifstream inputFileStream;
  this->OpenFileForReading( inputFileStream, m_FileName );

  // Find last position for tag search
  inputFileStream.seekg(5,std::ios::end);
  int endOfTagSearch = inputFileStream.tellg();

  inputFileStream.seekg(16,std::ios::beg); // skips header

  // let any exceptions propagate
  MetaDataDictionary & dico = this->GetMetaDataDictionary();

  unsigned short tag_shorts[2];
  unsigned int taglength;
  double offset1, offset2;
  while((inputFileStream.tellg()<endOfTagSearch-8)&&(inputFileStream.tellg()>-1))
  {
    inputFileStream.read(reinterpret_cast<char *>(&tag_shorts), sizeof(tag_shorts)); 
    inputFileStream.read(reinterpret_cast<char *>(&taglength), sizeof(taglength)); 

    Tag tag(tag_shorts[0],tag_shorts[1]);

    if(tag==PatientTag)
    {
      char patient_name_c_array[taglength]; 
      inputFileStream.read(reinterpret_cast<char *>(patient_name_c_array), taglength); 
      std::string tagString(tag);
      EncapsulateMetaData< std::string >( dico, tagString, std::string(patient_name_c_array) );
    }
    else if(tag==DimensionXTag && !m_isDoppler)
    {
      unsigned short dimension;
      inputFileStream.read(reinterpret_cast<char *>(&dimension), taglength); 
      this->SetDimensions(0,dimension);
    } 
    else if(tag==DimensionYTag && !m_isDoppler)
    {
      unsigned short dimension;
      inputFileStream.read(reinterpret_cast<char *>(&dimension), taglength); 
      this->SetDimensions(1,dimension);
    } 
    else if(tag==DimensionZTag && !m_isDoppler)
    {
      unsigned short dimension;
      inputFileStream.read(reinterpret_cast<char *>(&dimension), taglength); 
      this->SetDimensions(2,dimension);
    } 
    else if(tag==ResolutionTag && !m_isDoppler)
    {
      double value;
      inputFileStream.read(reinterpret_cast<char *>(&value), taglength); 
      this->m_Resolution = value *1000.0;
    } 
    else if(tag==Offset1Tag && !m_isDoppler)
    {
      double value;
      inputFileStream.read(reinterpret_cast<char *>(&value), taglength); 
      offset1 = value;
    } 
    else if(tag==Offset2Tag && !m_isDoppler)
    {
      double value;
      inputFileStream.read(reinterpret_cast<char *>(&value), taglength); 
      offset2 = value;
    } 
    else if(tag==AnglesPhiTag && !m_isDoppler)
    {
      int len = taglength/sizeof(double);
      double * angles = new double[len];
      inputFileStream.read(reinterpret_cast<char *>(angles), sizeof( double ) * len); 
      double amin = angles[0];
      double amax = angles[len-1];
      double bCentre = (amin+amax)/2;
      for(int i=0; i<len;i++){
        double angle = angles[i]-bCentre;
        this->m_TableAnglesPhi.push_back(std::make_pair(angle, i));
      }
      delete[] angles;
    } 
    else if(tag==AnglesThetaTag && !m_isDoppler)
    {
      int len = taglength/sizeof(double);
      double * angles = new double[len];
      inputFileStream.read(reinterpret_cast<char *>(angles), sizeof( double ) * len); 
      double amin = angles[0];
      double amax = angles[len-1];
      double bCentre = (amin+amax)/2;
      for(int i=0; i<len;i++){
        double angle = angles[i]-bCentre;
        this->m_TableAnglesTheta.push_back(std::make_pair(angle, i));
      }
      delete[] angles;
    } 

    else if(tag==DimensionXTagDoppler && m_isDoppler)
    {
      unsigned short dimension;
      inputFileStream.read(reinterpret_cast<char *>(&dimension), taglength); 
      this->SetDimensions(0,dimension);
    } 
    else if(tag==DimensionYTagDoppler && m_isDoppler)
    {
      unsigned short dimension;
      inputFileStream.read(reinterpret_cast<char *>(&dimension), taglength); 
      this->SetDimensions(1,dimension);
    } 
    else if(tag==DimensionZTagDoppler && m_isDoppler)
    {
      unsigned short dimension;
      inputFileStream.read(reinterpret_cast<char *>(&dimension), taglength); 
      this->SetDimensions(2,dimension);
    } 
    else if(tag==ResolutionTagDoppler && m_isDoppler)
    {
      double value;
      inputFileStream.read(reinterpret_cast<char *>(&value), taglength); 
      this->m_Resolution = value *1000.0;
    } 
    else if(tag==Offset1TagDoppler && m_isDoppler)
    {
      double value;
      inputFileStream.read(reinterpret_cast<char *>(&value), taglength); 
      offset1 = value;
    } 
    else if(tag==Offset2TagDoppler && m_isDoppler)
    {
      double value;
      inputFileStream.read(reinterpret_cast<char *>(&value), taglength); 
      offset2 = value;
    } 
    else if(tag==AnglesPhiTagDoppler && m_isDoppler)
    {
      int len = taglength/sizeof(double);
      double * angles = new double[len];
      inputFileStream.read(reinterpret_cast<char *>(angles), sizeof( double ) * len); 
      double amin = angles[0];
      double amax = angles[len-1];
      double bCentre = (amin+amax)/2;
      for(int i=0; i<len;i++){
        double angle = angles[i]-bCentre;
        this->m_TableAnglesPhi.push_back(std::make_pair(angle, i));
      }
      delete[] angles;
    } 
    else if(tag==AnglesThetaTagDoppler && m_isDoppler)
    {
      int len = taglength/sizeof(double);
      double * angles = new double[len];
      inputFileStream.read(reinterpret_cast<char *>(angles), sizeof( double ) * len); 
      double amin = angles[0];
      double amax = angles[len-1];
      double bCentre = (amin+amax)/2;
      for(int i=0; i<len;i++){
        double angle = angles[i]-bCentre;
        this->m_TableAnglesTheta.push_back(std::make_pair(angle, i));
      }
      delete[] angles;
    } 
    else 
    {
      inputFileStream.seekg(taglength,std::ios::cur);
    }

  }
  m_rBstart = offset1*m_Resolution;
  m_rD = -offset2*m_Resolution;

  inputFileStream.close();

}

bool KretzImageIO::CanWriteFile(const char *name)
{
  return false;
}

void KretzImageIO::WriteImageInformation()
{}

void KretzImageIO::Write(const void *buffer)
{
  {
    itkExceptionMacro(<< "Kretz file writing is not supported.");
  }
}

void KretzImageIO::PrintSelf(std::ostream & os, Indent indent) const
{
  Superclass::PrintSelf(os, indent);
  std::string value;
  const MetaDataDictionary & dico = this->GetMetaDataDictionary();
  ExposeMetaData< std::string >(dico, PatientTag, value);
  os << indent << "PatientName: " << value << std::endl;

}
} // end namespace itk



#ifndef __itkCartesianToToroidalTransform_h
#define __itkCartesianToToroidalTransform_h


#include <iostream>
#include "itkTransform.h"
#include "itkExceptionObject.h"
#include "itkMatrix.h"
#include <vector>
#include <itkArray.h>
#include <math.h>
#include <limits>


namespace itk
{

/** \brief Transformation from Cartesian to Toroidal space.
 *
 * Transforms three coordinates from cartesian coordinates to toroidal space <radius, theta, phi>. 
 * These are used in transforming the output of 3D ultrasound volumes back into scan-line data.
 *
 * \f[			r = \sqrt{ x^2 + (d - \frac{y}{sin( \phi )} ) } \f]
 * \f[			\phi = -tan^{-1}( \frac{y}{z-d} ) \f]
 * \f[			\theta = sin^{-1}( \frac{x}{b} ) \f]
 *
 *
 * where;
 *
 * \f$ \theta \f$ is the angle in BMode,
 * \f$ \phi \f$ is the angle the BMode is swept through,
 * \f$ r \f$ is the distance from the BMode focus
 * \f$ d \f$ is the distance between foci.
 *
 * Dimension must be 3 or an exception is thrown during transform.
 *
 * Extent of input (phi) should be only < 0, 2*pi ).
 *
 * \author Pádraig Looney, University of Oxford.
 *
 * \ingroup Transforms
 */
template <
        class TScalarType=double,          // Data type for scalars (float or double)
        unsigned int NDimensions=3>        // Number of dimensions
class ITK_EXPORT CartesianToToroidalTransform : public Transform< TScalarType, NDimensions, NDimensions >
{
public:

    /** Standard class typedefs. */
    typedef CartesianToToroidalTransform Self;
    typedef Transform< TScalarType, NDimensions, NDimensions > Superclass;
    typedef SmartPointer<Self>        Pointer;
    typedef SmartPointer<const Self>  ConstPointer;
    typedef std::vector<std::pair<double, double> > TableType;

    /** Variables specific to geometry*/
    double m_BModeRadius;
    double m_SweepRadius;
    double m_Resolution;

    std::vector<std::pair<double, double> > m_TableAnglesTheta;
    std::vector<std::pair<double, double> > m_TableAnglesPhi;

    /** New macro for creation of through the object factory.*/
    itkNewMacro( Self )

    itkSetMacro(BModeRadius, double)
    itkSetMacro(SweepRadius, double)
    itkSetMacro(Resolution, double)

    /** Run-time type information (and related methods). */
    itkTypeMacro( CartesianToToroidalTransform, Transform )

    /** Dimension of the domain space. */
    itkStaticConstMacro(SpaceDimension, unsigned int, NDimensions);
    itkStaticConstMacro(ParametersDimension, unsigned int, 0);

    /** Standard scalar type for this class. */
    typedef typename Superclass::ScalarType ScalarType;

    /** Standard Jacobian container. */
    typedef typename Superclass::JacobianType JacobianType;

    /** Standard vector type for this class. */
    typedef Vector<TScalarType, itkGetStaticConstMacro(SpaceDimension)>
    InputVectorType;
    typedef Vector<TScalarType, itkGetStaticConstMacro(SpaceDimension)>
    OutputVectorType;

    /** Standard covariant vector type for this class. */
    typedef CovariantVector<TScalarType, itkGetStaticConstMacro(SpaceDimension)>
    InputCovariantVectorType;
    typedef CovariantVector<TScalarType, itkGetStaticConstMacro(SpaceDimension)>
    OutputCovariantVectorType;

    /** Standard vnl_vector type for this class. */
    typedef vnl_vector_fixed<TScalarType, itkGetStaticConstMacro(SpaceDimension)>
    InputVnlVectorType;
    typedef vnl_vector_fixed<TScalarType, itkGetStaticConstMacro(SpaceDimension)>
    OutputVnlVectorType;

    /** Standard coordinate point type for this class. */
    typedef Point<TScalarType, itkGetStaticConstMacro(SpaceDimension)>
    InputPointType;
    typedef Point<TScalarType, itkGetStaticConstMacro(SpaceDimension)>
    OutputPointType;

    typedef typename Superclass::FixedParametersType FixedParametersType;
    typedef typename Superclass::ParametersType ParametersType;

    /** Method to transform a point.
     * This method transforms first two dimensions of a point from polar
     * coordinates <alpha,radius> to cartesian coordinates.
     */
    OutputPointType     TransformPoint(const InputPointType  &point ) const;

    /** Method to transform a vector - not applicable for this type of
      transform. */
    virtual OutputVectorType TransformVector(const InputVectorType &) const
    {
        itkExceptionMacro(<< "Method not applicable for polar transform." );
        return OutputVectorType();
    }

    /** Method to transform a vnl_vector - not applicable for this type of
      transform. */
    virtual OutputVnlVectorType TransformVector(const InputVnlVectorType &) const
    {
        itkExceptionMacro(<< "Method not applicable for polar transform. ");
        return OutputVnlVectorType();
    }

    /** Method to transform a CovariantVector - not applicable for this type of
      transform */
    virtual OutputCovariantVectorType TransformCovariantVector(
            const InputCovariantVectorType &) const
    {
        itkExceptionMacro(<< "Method not applicable for polar transfrom. ");
        return OutputCovariantVectorType();
    }

    /** Compute the Jacobian Matrix of the transformation at one point - not
      applicable for this type of transform */
    virtual void ComputeJacobianWithRespectToPosition( const InputPointType &, JacobianType &)  const
    {
        itkExceptionMacro(<< "Method not applicable for polar transform. ");
        //return this->m_Jacobian;
    }

    /** Compute the Jacobian Matrix of the transformation at one point - not
      applicable for this type of transform */
    virtual void ComputeJacobianWithRespectToParameters( const InputPointType &, JacobianType &)  const
    {
        itkExceptionMacro(<< "Method not applicable for polar transform. ");
        //return this->m_Jacobian;
    }

    virtual void SetTableTheta(const TableType);
    virtual void SetTablePhi(const TableType);

    virtual void SetParameters(const ParametersType &){

    }
    virtual void SetFixedParameters(const FixedParametersType &){

    }

protected:
    CartesianToToroidalTransform();
    ~CartesianToToroidalTransform();

    /** Print contents of an CartesianToToroidalTransform. */
    void PrintSelf(std::ostream &os, Indent indent) const;

private:
    CartesianToToroidalTransform(const Self&); //purposely not implemented
    void operator=(const Self&); //purposely not implemented
    virtual double Interpolate(double x, TableType table) const;

}; //class CartesianToToroidalTransform

}  // namespace itk
#ifndef ITK_MANUAL_INSTANTIATION
#include "itkCartesianToToroidalTransform.hxx"
#endif

#endif /* __itkCartesianToToroidalTransform_h */

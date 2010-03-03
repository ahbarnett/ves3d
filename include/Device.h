/**
 * @file   Device.h
 * @author Abtin Rahimian <arahimian@acm.org>
 * @date   Tue Feb 23 14:59:15 2010
 * 
 * @brief  The abstract class for the device class.
 */

#ifndef _DEVICE_H_
#define _DEVICE_H_

//Forward declaration for the friend function
template<typename T> class Device;

///The comparison operator for the device class
template<typename T, typename E>
inline bool operator==(const Device<T> &rhs, const Device<E> &lhs)
{
    return(rhs.GetID() == lhs.GetID());
}

unsigned long int AssignDeviceID();

///The enum types for the memory copying action.
enum MemcpyKind {MemcpyHostToHost, MemcpyHostToDevice, MemcpyDeviceToHost, MemcpyDeviceToDevice};

/**
 *  This class provides memory operations and also device specific
 *  optimized kernels <b>[Note the notational conventions in the
 *  Detailed Description section]</b>. The notational convention is
 *  that <tt>a</tt> and <tt>b</tt> are single scalars of type T;
 *  <tt>x</tt>, <tt>y</tt>, and <tt>z</tt> are scalar fields,
 *  i.e. arrays of scalar of type T with length
 *  <tt>stride*num_surfs</tt>; and <tt>u</tt>, <tt>v</tt>, and
 *  <tt>w</tt> are vector fields, i.e. arrays of scalar of type T with
 *  length <tt>3*stride*num_surfs</tt>. This convention is used
 *  throughout the class for both the naming of methods and naming the
 *  methods' parameters. All arrays are assumed to be residing on the
 *  device and there is no need of copying to the device etc.
 *
 *  @todo The tester does not cover the comparison operator, the ID'ing mechanism, and the sqrt
 */
template<typename T> class Device
{
  protected:
    ///The ID of the device
    const unsigned long int device_id_;

  public:
    ///The default constructor (takes care of id'ing).
    Device();
    
    ///Returns the ID of the device.
    unsigned long int GetID() const;
    
    friend unsigned long int AssignDeviceID();

    ///Memory allocation, since the type is defined as a template
    ///parameter, the size is different from the original malloc() as
    ///it <b>does not need to be multiplied by the sizeof(T)</b>.
    virtual T* Malloc(unsigned long int length) = 0;

    ///Freeing memory. 
    virtual void Free(T* ptr) = 0;

    ///Memory allocation and zero initialization for an array in
    ///memory, since the type is defined as a template parameter,
    ///there is no size argument.
    virtual T* Calloc(unsigned long int num) = 0;

    ///Copies the memory location form source to the destination. The
    ///kind of copy is from the enum type MemcpyKind and is <code>
    ///MemcpyHostToHost, MemcpyHostToDevice, MemcpyDeviceToHost, or
    ///MemcpyDeviceToDevice</code>.
    virtual T* Memcpy (T* destination, const T* source, unsigned long int num, enum MemcpyKind kind) = 0;

    ///Geometric dot product of two (Cartesian) vectors. 
    virtual T* DotProduct(const T* u_in, const T* v_in, int stride, int num_surfs, T* x_out) = 0;

    ///Geometric cross product of two (Cartesian) vectors.
    virtual T* CrossProduct(const T* u_in, const T* v_in, int stride, int num_surfs, T* w_out) = 0;

    ///Square root operator
    virtual T* Sqrt(const T* x_in, int stride, int num_surfs, T* sqrt_out) = 0;

    ///Element-wise inverse (of a scalar field).
    virtual T* xInv(const T* x_in, int stride, int num_surfs, T* xInv_out) = 0;

    ///Element-wise multiplication of scalar fields
    virtual T* xy(const T* x_in, const T* y_in, int stride, int num_surfs, T* xy_out) = 0;

    ///Element-wise division of scalar fields.
    virtual T* xyInv(const T* x_in, const T* y_in, int stride, int num_surfs, T* xyInv_out) = 0;
    
    ///Scaling and addition of an scalar field to another field.
    virtual T* axpy(T a_in, const T* x_in, const T* y_in, int stride, int num_surfs , T* axpy_out) = 0;
    
    ///Scaling of an array and addition of a single scalar. 
    virtual T* axpb(T a_in, const T*  x_in, T b_in, int stride, int num_surfs , T*  axpb_out) = 0;
    
    ///Element-wise scaling and addition.
    virtual T* xvpw(const T* x_in, const T*  v_in, const T*  w_in, int stride, int num_surfs, T*  xvpw_out) = 0;
    
    ///Element-wise scaling and addition.
    virtual T* xvpb(const T* x_in, const T*  v_in, T b_in, int stride, int num_surfs, T*  xvpb_out) = 0;

    ///The comparison operator for the device class
    template<typename T,typename E>
    friend bool operator==(const Device<T> &rhs, const Device<E> &lhs);
};

// Implementations
template<typename T>
Device<T>::Device() : 
    device_id_(AssignDeviceID()) 
{ }

template<typename T>
unsigned long int Device<T>::GetID() const
{
    return(device_id_);
}

///Returns a unique ID to the caller class. With this function we can
///use the same counter for all template parameters instantiation of
///the Device class.
unsigned long int AssignDeviceID()
{
    static unsigned long int device_counter = 0;
    return(device_counter++);
}

#endif //_DEVICE_H_
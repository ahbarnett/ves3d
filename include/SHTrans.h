#ifndef _SHTRANS_H_
#define _SHTRANS_H_

#include "Device.h"
#include "SHTMats.h"
#include <map>

template<typename T, enum DeviceType DT>
class SHTrans {
  private:
    Device<DT> *device_;
    SHTMats<T,DT> mats_;

    static const T alpha_ = (T) 1.0;
    static const T beta_  = (T) 0.0;

    int p;
    int dft_size;
        
    void DLT(T *trans, const T *inputs, T *outputs, int m, int n , 
        int k, int mf, int nf, int kf);
    void back(const T *inputs, T *work_arr, int n_funs, T *outputs, 
        T *trans, T *dft);

  public:
    SHTrans(Device<DT> *dev, int sh_order_in);
    ~SHTrans();

    void forward(const T *inputs, T *work_arr, int n_funs, T *outputs);
    void backward(const T *inputs, T *work_arr, int n_funs, T *outputs);
    void backward_du(const T *inputs, T *work_arr, int n_funs, T *outputs);
    void backward_dv(const T *inputs, T *work_arr, int n_funs, T *outputs);
    void backward_d2u(const T *inputs, T *work_arr, int n_funs, T *outputs);
    void backward_d2v(const T *inputs, T *work_arr, int n_funs, T *outputs);
    void backward_duv(const T *inputs, T *work_arr, int n_funs, T *outputs);
};

#include "SHTrans.cc"
#endif //_SHTRANS_H_

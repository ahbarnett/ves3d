template<typename T>
VesInteraction<T>::VesInteraction(InteractionFun_t interaction_handle,
    int num_threads) :
    interaction_handle_(interaction_handle),
    num_threads_(num_threads),
    each_thread_np_(new size_t[num_threads_]),
    each_thread_idx_(new size_t[num_threads_ + 1]),
    np_(0),
    containers_capacity_(0),
    all_pos_(NULL),
    all_den_(NULL),
    all_pot_(NULL)
{
    for(int ii=0; ii<num_threads_; ++ii)
        each_thread_idx_[ii] = each_thread_np_[ii] = 0;
}

template<typename T>
VesInteraction<T>::~VesInteraction()
{
    delete[] each_thread_np_;
    delete[] each_thread_idx_;

    delete[] all_pos_;
    delete[] all_den_;
    delete[] all_pot_;
}

template<typename T>
template<typename VecContainer>
Error_t VesInteraction<T>::operator()(
    const VecContainer &position, VecContainer &density,
    VecContainer &potential, void* usr_ptr) const
{
    typedef typename VecContainer::value_type value_type;

    if ( interaction_handle_ == NULL )
    {
        potential.getDevice().Memset(potential.begin(),
            0, potential.size() * sizeof(value_type));

        COUT("  No interaction."
            <<"\n ------------------------------------"<<std::endl);
        return ErrorEvent::NoInteraction;
    }

    //Getting the sizes
    size_t np(position.getNumSubs() * position.getStride());
    size_t n_cpy(position.size());
    size_t idx(this->getCpyDestIdx(np));

    //Copying to the host and maybe casting to fmm_value_type
    if(typeid(value_type) == typeid(T))
    {

        position.getDevice().Memcpy(all_pos_ + idx, position.begin(),
            n_cpy * sizeof(value_type),
            position.getDevice().MemcpyDeviceToHost);

        density.getDevice().Memcpy(all_den_ + idx, density.begin(),
            n_cpy * sizeof(value_type),
            density.getDevice().MemcpyDeviceToHost);
    }
    else
    {
        value_type* buffer(new value_type[n_cpy]);

        position.getDevice().Memcpy(buffer, position.begin(),
            n_cpy * sizeof(value_type),
            position.getDevice().MemcpyDeviceToHost);

        for(size_t ii=0; ii<n_cpy; ++ii)
            *(all_pos_ + idx + ii) = static_cast<T>(buffer[ii]);

        position.getDevice().Memcpy(buffer, density.begin(),
            n_cpy * sizeof(value_type),
            position.getDevice().MemcpyDeviceToHost);

        for(size_t ii=0; ii<n_cpy; ++ii)
            *(all_den_ + idx + ii) = static_cast<T>(buffer[ii]);

        delete[] buffer;
    }

    // call user interaction routine
#pragma omp barrier
#pragma omp master
    interaction_handle_(all_pos_, all_den_, np_, all_pot_, usr_ptr);
#pragma omp barrier

    //Copying back the potential to the device(s)
    if(typeid(value_type) == typeid(T))
    {
        potential.getDevice().Memcpy(potential.begin(), all_pot_ + idx,
            n_cpy * sizeof(value_type),
            potential.getDevice().MemcpyHostToDevice);
    }
    else
    {
        value_type* buffer(new value_type[n_cpy]);

        for(size_t ii=0; ii<n_cpy; ++ii)
            buffer[ii] = static_cast<T>(*(all_pot_ + idx + ii));

        potential.getDevice().Memcpy(potential.begin(), buffer,
            n_cpy * sizeof(value_type),
            potential.MemcpyHostToDevice);

        delete[] buffer;
    }

    COUT("  Interaction resolved."
        <<"\n ------------------------------------"<<std::endl);
    return ErrorEvent::Success;
}

template<typename T>
size_t VesInteraction<T>::getCpyDestIdx(size_t this_thread_np) const
{
    int threadNum = omp_get_thread_num();
    int runtimeNumThreads = omp_get_num_threads();
    each_thread_np_[threadNum] = this_thread_np;

#pragma omp barrier
    {
        if ( threadNum == 0 )
        {
            np_ = 0;
            int the_dim = 3;

            for(int ii=1; ii<=runtimeNumThreads; ++ii)
            {
                np_ += each_thread_np_[ii-1];
                each_thread_idx_[ii] = each_thread_idx_[ii-1] +
                    the_dim * each_thread_np_[ii-1];
            }
        }
    }

#pragma omp barrier
    {
        checkContainersSize();
    }

    return(each_thread_idx_[threadNum]);
}

template<typename T>
void VesInteraction<T>::checkContainersSize() const
{
    int runtimeNumThreads = omp_get_num_threads();
#pragma omp master
    {
        size_t new_capacity(each_thread_idx_[runtimeNumThreads]);

        if ( containers_capacity_ < new_capacity )
        {

            delete[] all_pos_;
            all_pos_ = new T[new_capacity];

            delete[] all_den_;
            all_den_ = new T[new_capacity];

            delete[] all_pot_;
            all_pot_ = new T[new_capacity];

            containers_capacity_ = new_capacity;
        }
    }
#pragma omp barrier
}

// template<typename Device>
// void template<typename T> VesInteraction<T>::directInteraction(const Device &device)
// {
//     cout<<"Direct"<<std::endl;
    //             int the_dim = VecContainer::getTheDim();

            //             position.getDevice().Transpose(all_pos_, np_, the_dim, all_pot_);
            //             position.getDevice().Transpose(all_den_, np_, the_dim, all_pos_);

            //             position.getDevice().DirectStokes(all_pot_, all_pos_, NULL,
            //                 np_, 1, all_pot_, 0, np_, all_den_);

            //             position.getDevice().Transpose(all_den_, the_dim, np_, all_pot_);
        //}
//         directInteraction(position.getDevice());

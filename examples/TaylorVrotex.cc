#include "Vectors.h"
#include "Parameters.h"
#include "Surface.h"
#include "VesInteraction.h"
#include "OperatorsMats.h"
#include "EvolveSurface.h"

#define DT CPU
extern const Device<CPU> the_device(0);
typedef float real;

template<typename VecContainer>
void TaylorVortex(const VecContainer &pos, const typename 
    VecContainer::value_type strength, VecContainer &vel_inf)
{
    assert( typeid(VecContainer::getDevice()) == typeid(Device<CPU>) );
    
    int n_surfs = pos.getNumSubs();
    int stride = pos.getStride();
    int idx;
    
    static VecContainer vp1, vp2;
    vp1.replicate(pos);
    vp2.replicate(pos);

    for ( size_t ii=0;ii<pos.size(); ++ii )
    {
        vp1.begin()[ii] = cos(pos.begin()[ii]);
        vp2.begin()[ii] = sin(pos.begin()[ii]); 
    }

    axpy(0.0, pos, vel_inf);
    for ( int ss=0; ss<n_surfs; ++ss )
        for ( int ii=0;ii<stride; ++ii)
        {
            idx = ss * DIM * stride + ii;
            vel_inf.begin()[idx          ] = - vp1.begin()[idx] * vp2.begin()[idx + stride];
            vel_inf.begin()[idx + stride ] = - vp2.begin()[idx] * vp1.begin()[idx + stride];

        }
    axpy(strength, vel_inf, vel_inf); 
}

int main(int argc, char **argv)
{
    COUT("\n\n ========================\n  Taylor Vortex: "
        <<"\n ========================"<<endl);

    typedef Parameters<real> Par_t;
    // Setting the parameters
    Par_t sim_par;
    sim_par.n_surfs = 2;   
    sim_par.ts = 1;    
    sim_par.time_horizon = 2;
    sim_par.scheme = Explicit;
    sim_par.bg_flow_param = 0.1;
    sim_par.rep_maxit = 20;
    sim_par.save_data = true;    
    sim_par.save_stride = 1;
    sim_par.save_file_name = "EvolveSurf.out";
    COUT(sim_par);
    
    //Cleaning the slate
    remove(sim_par.save_file_name.c_str());

    //Types
    typedef Scalars<real, DT, the_device> Sca_t;
    typedef Vectors<real, DT, the_device> Vec_t;

    typedef Surface<Sca_t,Vec_t> Sur_t;
    typedef VesInteraction<real> Interaction_t;
    typedef InterfacialVelocity<Sur_t, Interaction_t> IntVel_t;
    
    //IO
    DataIO myIO;
    
    //Initializing vesicle positions from text file
    Vec_t x0(sim_par.n_surfs, sim_par.sh_order);
    
    //reading the prototype form file
    myIO.ReadData("precomputed/dumbbell_cart12_single.txt",
        x0, 0, x0.getSubLength());
    
    //Making Centers And Populating The Prototype
    int nVec = sim_par.n_surfs;
    real* cntrs_host =  new real[nVec * DIM];
    for(int ii=0; ii<nVec; ++ii)
    {
        cntrs_host[DIM*ii    ] = 0;
        cntrs_host[DIM*ii + 1] = 3*ii;
        cntrs_host[DIM*ii + 2] = 0;
    }
    Array<real, DT, the_device> cntrs(DIM * sim_par.n_surfs);
    cntrs.getDevice().Memcpy(cntrs.begin(), cntrs_host,
        cntrs.size() * sizeof(real), MemcpyHostToDevice);
    Populate(x0, cntrs);
 
    //The Interaction Class
    Interaction_t Interaction(NULL);

    //Reading Operators From File
    bool readFromFile = true;
    OperatorsMats<Sca_t> Mats(readFromFile, sim_par);

    //Making The Surface, And Time Stepper
    Sur_t S(x0, Mats);
    Monitor<Sur_t> M(sim_par);
    RepartitionGateway<real> repart(NULL);

    IntVel_t F(S, Interaction, Mats, sim_par, &TaylorVortex);

//      EvolveSurface<Sur_t, Interaction_t> Es(Mats, sim_par, M, repart);
   
//     Es(S, Interaction);
// }
    
}


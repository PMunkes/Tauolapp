{
//look at higgs decays
Setup::decay_particle=25;
Setup::mass_power=1;
Setup::mass_scale_on=true;

// Setup histograms
int n_bins=60;
double default_min_bin=0.0;
double default_max_bin=1.1;
Setup::SetHistogramDefaults(n_bins,default_min_bin,default_max_bin);

// Setup User Histograms
Setup::UserTreeAnalysis = "CPtestUserTreeAnalysis";

// Description
Setup::gen1_desc_1="CP-test";
Setup::gen1_desc_2=" $H \\rightarrow 2 \\rho \\rho \\nu_{\\tau} \\bar{\\nu_{\\tau}} $";
Setup::gen1_desc_3="Benchmark file";

Setup::gen2_desc_1="CP-test";
Setup::gen2_desc_2=" $H \\rightarrow 2 \\rho \\rho \\nu_{\\tau} \\bar{\\nu_{\\tau}} $";
Setup::gen2_desc_3="New";


if (Setup::stage==0)
    printf("Setup loaded from SETUP.C, ANALYSIS stage.\n");
else 
    printf("Setup loaded from SETUP.C, GENERATION stage %i.\n",Setup::stage);

Setup::SuppressDecay(111); // suppress pi0 decays

};


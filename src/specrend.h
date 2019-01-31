#ifndef SPECREND_H
#define SPECREND_H

/* A colour system is defined by the CIE x and y coordinates of
   its three primary illuminants and the x and y coordinates of
   the white point. */

struct colourSystem {
    char *name;                     /* Colour system name */
    double xRed, yRed,              /* Red x, y */
           xGreen, yGreen,          /* Green x, y */
           xBlue, yBlue,            /* Blue x, y */
           xWhite, yWhite,          /* White point x, y */
           gamma;                   /* Gamma correction for system */
};

/* White point chromaticities. */

#define IlluminantC     0.3101, 0.3162          /* For NTSC television */
#define IlluminantD65   0.3127, 0.3291          /* For EBU and SMPTE */
#define IlluminantE     0.33333333, 0.33333333  /* CIE equal-energy illuminant */

/*  Gamma of nonlinear correction.

    See Charles Poynton's ColorFAQ Item 45 and GammaFAQ Item 6 at:

       http://www.poynton.com/ColorFAQ.html
       http://www.poynton.com/GammaFAQ.html

*/

#define GAMMA_REC709    0               /* Rec. 709 */

extern struct colourSystem NTSCsystem;
extern struct colourSystem EBUsystem;
extern struct colourSystem SMPTEsystem;
extern struct colourSystem HDTVsystem;
extern struct colourSystem CIEsystem;
extern struct colourSystem Rec709system;

extern double cie_colour_match[81][3];

extern double bbTemp;

void upvp_to_xy(double up, double vp, double *xc, double *yc);
void xy_to_upvp(double xc, double yc, double *up, double *vp);
void xyz_to_rgb(struct colourSystem *cs, double xc, double yc, double zc, double *r, double *g, double *b);
int inside_gamut(double r, double g, double b);
int constrain_rgb(double *r, double *g, double *b);
void gamma_correct(const struct colourSystem *cs, double *c);
void gamma_correct_rgb(const struct colourSystem *cs, double *r, double *g, double *b);
void norm_rgb(double *r, double *g, double *b);
void spectrum_to_xyz(double (*spec_intens)(double wavelength));
double bb_spectrum(double wavelength);

#endif

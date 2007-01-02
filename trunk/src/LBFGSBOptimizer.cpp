

/* Portions copyright (c) 2006 Stanford University and Jack Middleton.
 * Contributors:
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject
 * to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "Simmath_f2c.h"
#include "LBFGSBOptimizer.h"

using std::cout;
using std::endl;
int setulb_(integer *n, integer *m, doublereal *x, doublereal *l,
      doublereal *u, integer *nbd, doublereal *f, doublereal *g,
      doublereal *factr, doublereal *pgtol, doublereal *wa, integer *iwa,
      char *task, integer *iprint, char *csave, logical *lsave,
      integer *isave, doublereal *dsave, ftnlen task_len, ftnlen csave_len);

namespace SimTK {


// TODO make these options
static const int NUMBER_OF_CORRECTIONS = 5;
static double factr = 1.0e7;   // 


     LBFGSBOptimizer::LBFGSBOptimizer( OptimizerSystem& sys )
        : OptimizerRep( sys ) {
          int n,i;
          char buf[1024];

         n = sys.getNumParameters();

         if( n < 1 ) {
             char *where = "Optimizer Initialization";
             char *szName= "dimension";
             SimTK_THROW5(SimTK::Exception::ValueOutOfRange, szName, 1,  n, INT_MAX, where);
         }



          /* assume all paramters have both upper and lower limits */
          nbd = (int *)malloc(n*sizeof(int));
          for(i=0;i<n;i++) {
               nbd[i] = 2;
          }

         gradient = new double[n];

     } 

     double LBFGSBOptimizer::optimize(  Vector &results ) {

         int i;
         int run_optimizer = 1;
         char task[61];
         double f;
         int iprint = 1;
         int *iwa;
         char csave[61];
         logical lsave[4];
         int isave[44];
         double dsave[29];
         double *wa;
         double *lowerLimits, *upperLimits;
         const OptimizerSystem& sys = getOptimizerSystem();
         int n = sys.getNumParameters();
         int m = NUMBER_OF_CORRECTIONS;


         sys.getParameterLimits( &lowerLimits, &upperLimits );
         iwa = (int *)malloc(3*n*sizeof(int));
         wa = (double *)malloc( ((2*m + 4)*n + 12*m*m + 12*m)*sizeof(double));
         /* setup Numerical gradients  */

         strcpy( task, "START" );


         while( run_optimizer ) { 

            setulb_(&n, &m, &results[0], lowerLimits,
                    upperLimits, nbd, &f, gradient,
                    &factr, &convergenceTolerance, wa, iwa,
                    task, &iprint, csave, lsave, isave, dsave, 60, 60);

             if( strncmp( task, "FG", 2) == 0 ) {
                objectiveFuncWrapper( n, &results[0],  true, &f, (void*)this );
                gradientFuncWrapper( n,  &results[0],  false, gradient, (void*)this );
             } else if( strncmp( task, "NEW_X", 5) == 0 ){
                objectiveFuncWrapper( n, &results[0],  true, &f, (void*)this );
             } else {
                run_optimizer = 0;
                if( strncmp( task, "CONV", 4) != 0 ){
                    SimTK_THROW1(SimTK::Exception::OptimizerFailed , SimTK::String(task) ); 
                }
             }
         }

         return(f);
      }


} // namespace SimTK
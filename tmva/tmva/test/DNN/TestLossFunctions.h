// @(#)root/tmva $Id$
// Author: Simon Pfreundschuh

/*************************************************************************
 * Copyright (C) 2016, Simon Pfreundschuh                                *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

//////////////////////////////////////////////////////////////////////
// Generic tests of the loss functions                              //
//                                                                  //
// Contains generic test for architecture-specific implementations  //
// of the loss functions. Requires the architecture-specific matrix //
// type to be constructible and convertible from/to the             //
// TMatrixT<Double_t> type.                                         //
//////////////////////////////////////////////////////////////////////

#include "TMVA/DNN/Architectures/Reference.h"
#include "TMVA/DNN/Functions.h"
#include "TMVA/DNN/Net.h"
#include "Utility.h"

using namespace TMVA::DNN;

//______________________________________________________________________________
//
//  Mean Squared Error
//______________________________________________________________________________

template <typename Architecture>
auto testMeanSquaredError(size_t ntests)
-> typename Architecture::Scalar_t
{
   using Matrix_t = typename Architecture::Matrix_t;
   using Scalar_t   = typename Architecture::Scalar_t;
   Double_t maximumError = 0.0;

   for (size_t i = 0; i < ntests; i++) {
      size_t m = rand() % 100 + 1;
      size_t n = rand() % 100 + 1;

      TMatrixT<Double_t> X(m, n);
      TMatrixT<Double_t> Y(m, n);
      TMatrixT<Double_t> Z(m, n);

      randomMatrix(X);
      randomMatrix(Y);

      Matrix_t XArch(X);
      Matrix_t YArch(Y);

      Scalar_t mse = evaluate<Architecture>(ELossFunction::kMeanSquaredError,
                                            YArch, XArch);
      zipWithMatrix(Z, [](Scalar_t x, Scalar_t y){return x - y;}, X, Y);
      auto squaredSum = [](Scalar_t x, Scalar_t y){return x + y * y;};
      Scalar_t mseReference = reduceMean(squaredSum, 0.0, Z);

      Double_t error;
      if (mseReference != 0.0)
          error = std::fabs((mse - mseReference) / mseReference);
      else
          error = std::fabs(mse - mseReference);
      maximumError = std::max(error, maximumError);
   }
   return maximumError;
}

//______________________________________________________________________________
template <typename Architecture>
auto testMeanSquaredErrorGradients(size_t ntests)
-> typename Architecture::Scalar_t
{
   using Matrix_t = typename Architecture::Matrix_t;
   using Scalar_t   = typename Architecture::Scalar_t;
   Double_t maximumError = 0.0;

   for (size_t i = 0; i < ntests; i++) {
      size_t m = rand() % 100 + 1;
      size_t n = rand() % 100 + 1;

      TMatrixT<Double_t> X(m, n);
      TMatrixT<Double_t> Y(m, n);
      TMatrixT<Double_t> ZRef(m, n);

      randomMatrix(X);
      randomMatrix(Y);

      Matrix_t XArch(X);
      Matrix_t YArch(Y);
      Matrix_t ZArch(Y);

      evaluateGradients<Architecture>(ZArch, ELossFunction::kMeanSquaredError,
                                     XArch, YArch);
      auto normedDifference = [m, n](Scalar_t x, Scalar_t y) {
         return 2.0 * (y - x) / (m * n);
      };
      zipWithMatrix(ZRef, normedDifference, X, Y);
      TMatrixT<Double_t> Z(ZArch);
      Double_t error = maximumRelativeError(Z, ZRef);
      maximumError = std::max(error, maximumError);
   }
   return maximumError;
}

//______________________________________________________________________________
//
//  Cross Entropy
//______________________________________________________________________________

template <typename Architecture>
auto testCrossEntropy(size_t ntests)
-> typename Architecture::Scalar_t
{
   using Matrix_t = typename Architecture::Matrix_t;
   using Scalar_t   = typename Architecture::Scalar_t;
   Double_t maximumError = 0.0;

   for (size_t i = 0; i < ntests; i++) {
      size_t m = rand() % 100 + 1;
      size_t n = rand() % 100 + 1;

      TMatrixT<Double_t> X(m, n);
      TMatrixT<Double_t> Y(m, n);
      TMatrixT<Double_t> Z(m, n);

      randomMatrix(X);
      randomMatrix(Y);

      Matrix_t XArch(X);
      Matrix_t YArch(Y);

      Scalar_t ce = evaluate<Architecture>(ELossFunction::kCrossEntropy,
                                           YArch, XArch);

      auto crossCorrelation = [](Scalar_t x, Scalar_t y) {
         Scalar_t sig = 1.0 / (1.0 + std::exp(-x));
            return y * std::log(sig) + (1 - y) * std::log(1 - sig);
      };
      zipWithMatrix(Z, crossCorrelation, X, Y);
      auto sum = [](Scalar_t x, Scalar_t y) {return x + y;};
      Scalar_t ceReference = - reduceMean(sum, 0.0, Z);

      Double_t error;
      if (ceReference != 0.0)
          error = std::fabs((ce - ceReference) / ceReference);
      else
          error = std::fabs(ce - ceReference);
      maximumError = std::max(error, maximumError);
   }
   return maximumError;
}

//______________________________________________________________________________
template <typename Architecture>
auto testCrossEntropyGradients(size_t ntests)
-> typename Architecture::Scalar_t
{
   using Matrix_t = typename Architecture::Matrix_t;
   using Scalar_t   = typename Architecture::Scalar_t;
   Double_t maximumError = 0.0;

   for (size_t i = 0; i < ntests; i++) {
      size_t m = 8; //rand() % 100 + 1;
      size_t n = 8; //rand() % 100 + 1;

      TMatrixT<Double_t> X(m, n);
      TMatrixT<Double_t> Y(m, n);
      TMatrixT<Double_t> ZRef(m, n);

      randomMatrix(X);
      randomMatrix(Y);

      Matrix_t XArch(X);
      Matrix_t YArch(Y);
      Matrix_t ZArch(Y);

      evaluateGradients<Architecture>(ZArch, ELossFunction::kCrossEntropy,
                                     YArch, XArch);
      auto crossCorrelationGradient = [m, n](Scalar_t x, Scalar_t y) {
         Scalar_t sig  = 1.0 / (1.0 + std::exp(-x));
         Scalar_t norm = 1.0 / ((Scalar_t) m * n);
         return (sig - y) * norm;};
      zipWithMatrix(ZRef, crossCorrelationGradient, X, Y);

      TMatrixT<Double_t> Z(ZArch);
      Double_t error = maximumRelativeError(Z, ZRef);
      maximumError = std::max(error, maximumError);
   }
   return maximumError;
}

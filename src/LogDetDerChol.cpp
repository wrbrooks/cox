#include <Rcpp.h>
#include <RcppEigen.h>

using namespace Eigen;
using namespace Rcpp;
using namespace std;

// [[Rcpp::depends(RcppEigen)]]

// This function computes the derivative of the log determinant of a Cholesky factor
// with respect to regression coefficients beta and precision component tau
//' @export
// [[Rcpp::export]]
NumericVector LogDetDerChol(const Eigen::MatrixXd L, const Eigen::MatrixXd S, const Eigen::MatrixXd X, const Eigen::VectorXd mu, const Eigen::VectorXd wt, double tau)
{
  int r = S.cols();
  int p = X.cols();

  // dM/d(beta): the change in the Hessian matrix with change in beta.
  Eigen::MatrixXd *gr = new Eigen::MatrixXd[p];
  for (int i=0; i<p; i++)
    gr[i] = Eigen::MatrixXd::Zero(r, r);

  for (int j=0; j<p; j++)
  {
    VectorXd Xcol = X.col(j);
    Eigen::VectorXd diag = wt.array() * mu.array() * Xcol.array();
    gr[j] = S.transpose() * diag.asDiagonal() * S;
  }

  NumericVector dL(p+1);
  Eigen::MatrixXd F;
  F = Eigen::MatrixXd::Zero(r, r);
  for (int l=0; l<r; l++)
    F(l,l) = 1/L(l,l);


  //if (verbose) cat('.')

  //-----------------------------------------
  // Calculate the derivative of the log-determinant of the Hessian's Cholesky factorization, following reverse-differentiation steps on page 138 of
  // S.P. Smith paper (Journal of Computational and Graphical Statistics, 1995, 4(2)):

  for(int k=r-2; k>=0; k--) {
    for(int j=k+1; j<r; j++) {
      for(int i=j; i<r; i++) {
        F(i,k) -= F(i,j) * L(j,k);
        F(j,k) -= F(i,j) * L(i,k);
      }
      F(j,k) = F(j,k) / L(k, k);
      F(k,k) -= L(j,k) * F(j,k);
    }
    F(k,k) = F(k,k) / L(k,k) / 2;
  }

  dL(p) = tau * F.diagonal().sum();
  for (int j=0; j<p; j++)
    dL(j) = (F.array()*gr[j].array()).sum();

  return(dL);
}

.. _robust-optimization-bibliography:

Bibliographic notes and references
==================================

.. currentmodule:: otrobopt

Bibliographic notes
-------------------

The literature review of RBDO led to the following design choices.  The
double-loop formulation with RIA or PMA is the reference implementation
[Aoues2010]_, [Tu1999]_, [Valdebenito2010]_.  SORA decouples the loops through
a shift of the deterministic constraints [Du2004]_, and the single-loop
methods replace the inner reliability problem by a fixed-point update
[Liang2004]_.  The sample-average and scenario approximations are
distribution-free references [Shapiro2009]_, [Calafiore2006]_,
[Rocchetta2021]_, while joint chance constraints over finite distributions
admit exact mixed-integer formulations [Luedtke2010]_, [Ahmed2018]_.
Analytic approximations and gradients for joint chance constraints are
available for specific families [Geletu2019]_, [Xie2019]_,
[VanAckooij2014]_, and sequential convex approaches require additional
structure [Hong2011]_.

The classical robust optimization background is in [BenTal2009]_,
[Beyer2007]_ and [Bertsimas2004]_.  Reliability-based design and its
benchmarks are covered by [Aoues2010]_, [Valdebenito2010]_, [Du2004]_,
[Tu1999]_, [Liang2004]_, [Lemaire2009]_, [Ditlevsen1996]_, [Hasofer1974]_
and [Rackwitz1978]_.  Chance constraints and stochastic programming are
treated in [Prekopa1995]_, [Shapiro2003]_, [Shapiro2009]_,
[Nemirovski2006]_, [Chen2010]_, [Rockafellar2000]_ and [Calafiore2006]_.
The isoprobabilistic constructions are due to [Rosenblatt1952]_,
[Nataf1962]_, [Hohenbichler1981]_, [Lebrun2009a]_, [Lebrun2009b]_ and
[Lebrun2009c]_.  Measure-theoretic and Monte Carlo foundations are in
[Billingsley1995]_, [Bogachev2007]_ and [Glasserman2003]_.  The
reparameterization identity used by stochastic optimization is discussed by
[Kingma2014]_ and [Rezende2014]_.

References
----------

.. [Aoues2010] A. Aoues and D. Chateauneuf. Benchmark study of numerical
   methods for reliability-based design optimization. *Structural and
   Multidisciplinary Optimization*, 41(2):277--294, 2010.

.. [BenTal2009] A. Ben-Tal, L. El Ghaoui and A. Nemirovski.
   *Robust Optimization*. Princeton University Press, 2009.


.. [Bogachev2007] V. I. Bogachev. *Measure Theory*. Springer, 2007.

.. [Chen2010] W. Chen, M. Sim, J. Sun and C.-P. Teo. From CVaR to uncertainty
   set: implications in joint chance-constrained optimization. *Operations
   Research*, 58(2):470--485, 2010.

.. [Ditlevsen1996] O. Ditlevsen and H. O. Madsen. *Structural Reliability
   Methods*. Wiley, 1996.

.. [Du2004] X. Du and W. Chen. Sequential optimization and reliability
   assessment method for probabilistic optimization. *Structural and
   Multidisciplinary Optimization*, 27(2):120--130, 2004.

.. [Glasserman2003] P. Glasserman. *Monte Carlo Methods in Financial
   Engineering*. Springer, 2003.

.. [Hasofer1974] A. M. Hasofer and N. C. Lind. Exact and invariant second
   moment code format. *Journal of the Engineering Mechanics Division*,
   100(1):111--121, 1974.

.. [Hohenbichler1981] M. Hohenbichler and R. Rackwitz. Non-normal dependent
   vectors in structural reliability. *Journal of Engineering Mechanics*,
   107(6):1227--1238, 1981.

.. [Kingma2014] D. P. Kingma and M. Welling. Auto-encoding variational Bayes.
   In *International Conference on Learning Representations*, 2014.

.. [Lebrun2009a] R. Lebrun and A. Dutfoy. An innovating analysis of the Nataf
   transformation from the copula viewpoint. *Probabilistic Engineering
   Mechanics*, 24(3):312--320, 2009.

.. [Lebrun2009b] R. Lebrun and A. Dutfoy. A generalization of the Nataf
   transformation to distributions with elliptical copula. *Probabilistic
   Engineering Mechanics*, 24(2):172--178, 2009.

.. [Lebrun2009c] R. Lebrun and A. Dutfoy. Do Rosenblatt and Nataf
   isoprobabilistic transformations really differ? *Probabilistic Engineering
   Mechanics*, 24(4):577--584, 2009.

.. [Lemaire2009] M. Lemaire, A. Chateauneuf and J.-C. Mitteau. *Structural
   Reliability and Optimal Design*. Springer, 2009.

.. [Liang2004] J. Liang, Z. P. Mourelatos and J. Tu. A single-loop method for
   reliability-based design optimization. In *Proceedings of the ASME Design
   Engineering Technical Conferences*, 2004.

.. [Nataf1962] A. Nataf. Determination des distributions de probabilites dont
   les marges sont donnees. *Comptes Rendus de l'Academie des Sciences*,
   255:42--43, 1962.

.. [Nemirovski2006] A. Nemirovski and A. Shapiro. Convex approximations of
   chance constrained programs. *SIAM Journal on Optimization*,
   17(4):969--996, 2006.

.. [Prekopa1995] A. Prekopa. *Stochastic Programming*. Kluwer, 1995.

.. [Rackwitz1978] R. Rackwitz and B. Fiessler. Structural reliability under
   combined random load sequences. *Computers & Structures*, 9(5):489--494,
   1978.

.. [Rezende2014] D. J. Rezende, S. Mohamed and D. Wierstra. Stochastic
   backpropagation and approximate inference in deep latent Gaussian models.
   In *International Conference on Machine Learning*, 2014.

.. [Rockafellar2000] R. T. Rockafellar and S. Uryasev. Optimization of
   conditional value-at-risk. *Journal of Risk*, 2(3):21--41, 2000.

.. [Rosenblatt1952] M. Rosenblatt. Remarks on a multivariate transformation.
   *Annals of Mathematical Statistics*, 23(3):470--472, 1952.

.. [Shapiro2003] A. Shapiro. Monte Carlo sampling methods. In A. Ruszczynski
   and A. Shapiro, editors, *Stochastic Programming*, Handbooks in Operations
   Research and Management Science, pages 353--425. Elsevier, 2003.

.. [Shapiro2009] A. Shapiro, D. Dentcheva and A. Ruszczynski. *Lectures on
   Stochastic Programming*. SIAM, 2009.

.. [Tu1999] J. Tu, K. K. Choi and Y. H. Park. A new study on reliability-based
   design optimization. *Journal of Mechanical Design*, 121(4):557--564,
   1999.

.. [Valdebenito2010] M. A. Valdebenito and G. I. Schueller. A survey on
   approaches for reliability-based optimization. *Structural and
   Multidisciplinary Optimization*, 42(1):1--24, 2010.

.. [Beyer2007] H.-G. Beyer and B. Sendhoff. Robust optimization -- a
   comprehensive survey. *Computer Methods in Applied Mechanics and
   Engineering*, 196(33--34):3190--3218, 2007.

.. [Bertsimas2004] D. Bertsimas and M. Sim. The price of robustness.
   *Operations Research*, 52(1):35--53, 2004.

.. [Calafiore2006] G. C. Calafiore and M. C. Campi. The scenario approach to
   robust control design. *IEEE Transactions on Automatic Control*,
   51(5):742--752, 2006.

.. [Billingsley1995] P. Billingsley. *Probability and Measure*. Wiley, 1995.

.. [Ahmed2018] S. Ahmed and W. Xie. Relaxations and approximations of chance
   constraints under finite distributions. *Mathematical Programming*,
   170(1):43--65, 2018.

.. [Geletu2019] A. Geletu, A. Hoffmann and P. Li. Analytic approximation and
   differentiability of joint chance constraints. *Optimization*,
   68(10):1985--2023, 2019.

.. [Hong2011] L. J. Hong, Y. Yang and L. Zhang. Sequential convex
   approximations to joint chance constrained programs: a Monte Carlo
   approach. *Operations Research*, 59(3):617--630, 2011.

.. [Luedtke2010] J. Luedtke, S. Ahmed and G. Nemhauser. An integer programming
   approach for linear programs with probabilistic constraints.
   *Mathematical Programming*, 122(2):247--272, 2010.

.. [Rocchetta2021] R. Rocchetta and L. G. Crespo. A scenario optimization
   approach to reliability-based and risk-based design: soft-constrained
   modulation of failure probability bounds. *Reliability Engineering and
   System Safety*, 216:107900, 2021.

.. [VanAckooij2014] W. van Ackooij and R. Henrion. Gradient formulae for
   nonlinear probabilistic constraints with Gaussian and Gaussian-like
   distributions. *SIAM Journal on Optimization*, 24(4):1864--1889, 2014.

.. [Xie2019] W. Xie, S. Ahmed and R. Jiang. Optimized Bonferroni
   approximations of distributionally robust joint chance constraints.
   *Mathematical Programming*, 191(1):79--112, 2022.

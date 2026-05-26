%feature("docstring") OTROBOPT::InverseFORM
R"RAW(Inverse First Order Reliability Method.

The Inverse FORM algorithm solves the inverse reliability problem:
given a target reliability index :math:`\beta_t`, find the parameter
:math:`\theta` of the limit state function :math:`g(\ux, \theta)` such
that the reliability index of the resulting failure event equals
:math:`\beta_t`.

Formally, we seek :math:`\theta` such that:

.. math::

    \beta(\theta) = \beta_t

where :math:`\beta(\theta)` is the Hasofer-Lind reliability index
associated with the failure event :math:`\{g(\ux, \theta) \leq 0\}`.

The algorithm is iterative. At each iteration :math:`k`:

1. Compute the gradient of the limit state function with respect to the
   physical variables :math:`\nabla_\ux g` and with respect to the
   parameter :math:`\frac{\partial g}{\partial \theta}`.
2. Compute the direction of update in the standard space:

   .. math::

       \Delta \ux = -\frac{\beta_t}{\|\nabla_\ux g\|} \nabla_\ux g - \ux

3. Compute the parameter update:

   .. math::

       \Delta \theta = \frac{\ux \cdot \nabla_\ux g - g + \beta_t \|\nabla_\ux g\|}
                              {\frac{\partial g}{\partial \theta}}

4. Update :math:`\ux \leftarrow \ux + \alpha \Delta \ux` and
   :math:`\theta \leftarrow \theta + \alpha \Delta \theta` where
   :math:`\alpha` is determined by a step-size control strategy.

The algorithm converges when:

- The parameter variation :math:`|\Delta \theta|` is below :math:`\varepsilon_\theta` (variable convergence)
- The limit state value :math:`|g(\ux, \theta)|` is below :math:`\varepsilon_g` (limit state convergence)
- The beta variation :math:`|\|\ux\| - |\beta_t||` is below :math:`\varepsilon_\beta` (beta convergence)


Parameters
----------
event : :class:`~openturns.RandomVector`
    Event we are computing the probability of.
parameterName : str
    Name of the parameter of the limit state function to be calibrated.
physicalStartingPoint : sequence of float
    Physical space starting point for the design point search.

Examples
--------
>>> import openturns as ot
>>> import otrobopt
>>> ot.RandomGenerator.SetSeed(0)

Define a simple beam deflection limit state with a parametric input:

>>> full = ot.SymbolicFunction(['E', 'F', 'L', 'b', 'h'], ['F*L^3/(48.*E*b*h^3/12.)'])
>>> g = ot.ParametricFunction(full, [2], [5.0])  # freeze L = 5.0

Define the random distribution for the inputs:

>>> coll = [ot.LogNormalMuSigmaOverMu(30000., 0.12, 0.).getDistribution(),
...         ot.LogNormalMuSigmaOverMu(0.1, 0.2, 0.).getDistribution(),
...         ot.LogNormalMuSigmaOverMu(0.2, 0.05, 0.).getDistribution(),
...         ot.LogNormalMuSigmaOverMu(0.4, 0.05, 0.).getDistribution()]
>>> distribution = ot.JointDistribution(coll)
>>> x0 = [coll[i].computeQuantile(0.5)[0] for i in range(len(coll))]
>>> vect = ot.RandomVector(distribution)
>>> output = ot.CompositeRandomVector(g, vect)
>>> event = ot.ThresholdEvent(output, ot.Greater(), 0.02)

Solve the inverse problem: what beam length :math:`L` yields
:math:`\beta = 3.0`?

>>> algo = otrobopt.InverseFORM(event, 'L', x0)
>>> algo.setTargetBeta(3.0)
>>> algo.run()
>>> result = algo.getResult()
>>> parameter = result.getParameter()

See also
--------
otrobopt.InverseFORMResult, otrobopt.SubsetInverseSampling)RAW"

// ---------------------------------------------------------------------------

%feature("docstring") OTROBOPT::InverseFORM::getResult
"Result accessor.

Returns
-------
result : :class:`~otrobopt.InverseFORMResult`
    The result structure containing the calibrated parameter, convergence
    criteria, and the standard FORM results."

// ---------------------------------------------------------------------------

%feature("docstring") OTROBOPT::InverseFORM::setResult
"Result accessor.

Parameters
----------
result : :class:`~otrobopt.InverseFORMResult`
    The result structure."

// ---------------------------------------------------------------------------

%feature("docstring") OTROBOPT::InverseFORM::run
"Run the algorithm.

Launches the iterative inverse FORM algorithm. The result is stored
internally and can be retrieved with :meth:`getResult`."

// ---------------------------------------------------------------------------

%feature("docstring") OTROBOPT::InverseFORM::setTargetBeta
R"RAW(Target reliability index accessor.

Parameters
----------
beta : float
    Target reliability index :math:`\beta_t`.)RAW"

// ---------------------------------------------------------------------------

%feature("docstring") OTROBOPT::InverseFORM::getTargetBeta
R"RAW(Target reliability index accessor.

Returns
-------
beta : float
    Target reliability index :math:`\beta_t`.)RAW"

// ---------------------------------------------------------------------------

%feature("docstring") OTROBOPT::InverseFORM::setFixedStep
R"RAW(Fixed-step mode accessor.

When enabled, the algorithm uses a constant step size
:math:`\alpha` (see :meth:`setFixedStepValue`) for the parameter
and design point updates. When disabled, an Armijo-like line search is
performed at each iteration.

Parameters
----------
fixedStep : bool
    Whether to use a fixed step size.)RAW"

// ---------------------------------------------------------------------------

%feature("docstring") OTROBOPT::InverseFORM::getFixedStep
R"RAW(Fixed-step mode accessor.

Returns
-------
fixedStep : bool
    Whether a fixed step size is used.)RAW"

// ---------------------------------------------------------------------------

%feature("docstring") OTROBOPT::InverseFORM::setFixedStepValue
R"RAW(Fixed-step value accessor.

Parameters
----------
step : float
    Step size :math:`\alpha` used when the fixed-step mode is enabled.)RAW"

// ---------------------------------------------------------------------------

%feature("docstring") OTROBOPT::InverseFORM::getFixedStepValue
R"RAW(Fixed-step value accessor.

Returns
-------
step : float
    Step size :math:`\alpha` used when the fixed-step mode is enabled.)RAW"

// ---------------------------------------------------------------------------

%feature("docstring") OTROBOPT::InverseFORM::setVariableStepMaxIterations
R"RAW(Line search maximum iteration accessor.

Maximum number of backtracking steps in the variable step-size
(Armijo-like) line search.

Parameters
----------
maxIter : int
    Maximum number of line search iterations.)RAW"

// ---------------------------------------------------------------------------

%feature("docstring") OTROBOPT::InverseFORM::getVariableStepMaxIterations
R"RAW(Line search maximum iteration accessor.

Returns
-------
maxIter : int
    Maximum number of line search iterations.)RAW"

// ---------------------------------------------------------------------------

%feature("docstring") OTROBOPT::InverseFORM::setMaximumIteration
"Maximum iteration accessor.

Parameters
----------
maxIter : int
    Maximum number of outer iterations of the inverse FORM algorithm."

// ---------------------------------------------------------------------------

%feature("docstring") OTROBOPT::InverseFORM::getMaximumIteration
"Maximum iteration accessor.

Returns
-------
maxIter : int
    Maximum number of outer iterations."

// ---------------------------------------------------------------------------

%feature("docstring") OTROBOPT::InverseFORM::setBetaConvergence
R"RAW(Beta convergence criterion accessor.

Sets the tolerance :math:`\varepsilon_\beta` on the convergence of the
reliability index :math:`\beta`.

Parameters
----------
betaConv : float
    Convergence threshold on :math:`|\|\ux\| - |\beta_t||`.)RAW"

// ---------------------------------------------------------------------------

%feature("docstring") OTROBOPT::InverseFORM::getBetaConvergence
R"RAW(Beta convergence criterion accessor.

Returns
-------
betaConv : float
    Convergence threshold on :math:`|\|\ux\| - |\beta_t||`.)RAW"

// ---------------------------------------------------------------------------

%feature("docstring") OTROBOPT::InverseFORM::setVariableConvergence
R"RAW(Variable convergence criterion accessor.

Sets the tolerance :math:`\varepsilon_\theta` on the convergence of the
calibrated parameter.

Parameters
----------
varConv : float
    Convergence threshold on :math:`|\Delta \theta|`.)RAW"

// ---------------------------------------------------------------------------

%feature("docstring") OTROBOPT::InverseFORM::getVariableConvergence
R"RAW(Variable convergence criterion accessor.

Returns
-------
varConv : float
    Convergence threshold on :math:`|\Delta \theta|`.)RAW"

// ---------------------------------------------------------------------------

%feature("docstring") OTROBOPT::InverseFORM::setLimitStateConvergence
R"RAW(Limit-state convergence criterion accessor.

Sets the tolerance :math:`\varepsilon_g` on the convergence of the
limit state function value.

Parameters
----------
gConv : float
    Convergence threshold on :math:`|g(\ux, \theta)|`.)RAW"

// ---------------------------------------------------------------------------

%feature("docstring") OTROBOPT::InverseFORM::getLimitStateConvergence
R"RAW(Limit-state convergence criterion accessor.

Returns
-------
gConv : float
    Convergence threshold on :math:`|g(\ux, \theta)|`.)RAW"

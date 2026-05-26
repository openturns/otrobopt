%feature("docstring") OTROBOPT::InverseFORMResult
R"RAW(Result of the Inverse FORM algorithm.

This class stores the outcome of an :class:`~otrobopt.InverseFORM`
analysis. It inherits from :class:`~openturns.FORMResult` and provides
additional accessors for the calibrated parameter, its description, and
the convergence criteria.

It contains:

- The standard FORM results obtained at the calibrated parameter value,
  including the design point in standard and physical spaces, the
  Hasofer-Lind reliability index, and the sensitivity measures.
- The calibrated parameter value :math:`\theta^*`.
- The convergence criteria values:
    * Number of iterations,
    * Final parameter variation :math:`|\Delta \theta|`,
    * Final limit state value :math:`|g(\ux, \theta)|`,
    * Final beta variation :math:`|\|\ux\| - |\beta_t||`.

Examples
--------
>>> import openturns as ot
>>> import otrobopt
>>> ot.RandomGenerator.SetSeed(0)
>>> full = ot.SymbolicFunction(['E', 'F', 'L', 'b', 'h'], ['F*L^3/(48.*E*b*h^3/12.)'])
>>> g = ot.ParametricFunction(full, [2], [5.0])
>>> coll = [ot.LogNormalMuSigmaOverMu(30000., 0.12, 0.).getDistribution(),
...         ot.LogNormalMuSigmaOverMu(0.1, 0.2, 0.).getDistribution(),
...         ot.LogNormalMuSigmaOverMu(0.2, 0.05, 0.).getDistribution(),
...         ot.LogNormalMuSigmaOverMu(0.4, 0.05, 0.).getDistribution()]
>>> distribution = ot.JointDistribution(coll)
>>> x0 = [coll[i].computeQuantile(0.5)[0] for i in range(len(coll))]
>>> vect = ot.RandomVector(distribution)
>>> output = ot.CompositeRandomVector(g, vect)
>>> event = ot.ThresholdEvent(output, ot.Greater(), 0.02)
>>> algo = otrobopt.InverseFORM(event, 'L', x0)
>>> algo.run()
>>> result = algo.getResult()
>>> parameter = result.getParameter()
>>> description = result.getParameterDescription()
>>> criteria = result.getConvergenceCriteria()
>>> beta = result.getHasoferReliabilityIndex()

See also
--------
otrobopt.InverseFORM, openturns.FORMResult)RAW"

// ---------------------------------------------------------------------------

%feature("docstring") OTROBOPT::InverseFORMResult::getParameter
R"RAW(Parameter accessor.

Returns the calibrated value of the limit state function parameter.

Returns
-------
parameter : :py:class:`openturns.Point`
    Calibrated parameter value :math:`\theta^*`.)RAW"

// ---------------------------------------------------------------------------

%feature("docstring") OTROBOPT::InverseFORMResult::getParameterDescription
"Parameter description accessor.

Returns the description (name) of the calibrated parameter.

Returns
-------
description : :py:class:`openturns.Description`
    Description of the calibrated parameter."

// ---------------------------------------------------------------------------

%feature("docstring") OTROBOPT::InverseFORMResult::getConvergenceCriteria
R"RAW(Convergence criteria accessor.

Returns a point containing the convergence criteria values at the end of
the algorithm:

- :math:`[0]` : number of iterations,
- :math:`[1]` : final parameter variation :math:`|\Delta \theta|`,
- :math:`[2]` : final limit state value :math:`|g(\ux, \theta)|`,
- :math:`[3]` : final beta variation :math:`|\|\ux\| - |\beta_t||`.

Returns
-------
criteria : :py:class:`openturns.Point`
    Convergence criteria values.)RAW"

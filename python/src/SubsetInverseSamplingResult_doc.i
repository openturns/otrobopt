%feature("docstring") OTROBOPT::SubsetInverseSamplingResult
R"RAW(Result of the subset inverse simulation.

Stores the outcome of a :class:`~otrobopt.SubsetInverseSampling`
analysis. Inherits from :class:`~openturns.ProbabilitySimulationResult`
and adds the coefficient of variation of the threshold estimate.

Notes
-----
The result provides access to:

- The probability estimate and its variance (inherited)
- The coefficient of variation of the estimate
- The number of outer sampling steps and block size (inherited)
- The event that was simulated (inherited)

The threshold value at the target probability is obtained from the
algorithm via :meth:`~otrobopt.SubsetInverseSampling.getThresholdPerStep`.

Parameters
----------
event : :class:`~openturns.RandomVector`
    The event that was simulated.

See also
--------
otrobopt.SubsetInverseSampling, openturns.ProbabilitySimulationResult)RAW"

Examples
========

Example 1
---------

This example is a simple analytic function.

1- Problem statement
````````````````````
.. math::

    

a- Objective
''''''''''''

The parametric function :math:`J(x, \theta)` is subject to minimization

.. math::

    J(x, \theta) = 15.0 * ((x1+\theta_1)^2 + (x2+\theta_2)^2) - 100.0 * \exp(-5. * ((x1 + \theta_1 + 1.6)^2+(x2 + \theta_2 + 1.6)^2))

    \theta_1, \theta_2 \sim N(0, 0.1)

    -3 \leq x_1, x2 \leq 3

b- Constraints
''''''''''''''

.. math::

    g_1(x, \theta) = (x1+\theta_1 - 0.5)^2 + (x2+\theta_2)^2 - 4.0 \leq 0

    g_2(x, \theta) = (x1+\theta_1 + 0.5)^2 + (x2+\theta_2)^2 - 4.0 \leq 0



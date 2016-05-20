Example 5 : Mean measure
========================

1- Problem statement
--------------------

Note `f` the parametric function defined by:
 
.. math::

    \begin{aligned}
    & f(x, \theta)=x \theta
    \end{aligned}

`x` and :math:`\theta` being respectively the variable and parameter.

The goal is to compute the mean measure of `f` assuming the random variable :math:`\Theta` follows a distribution :math:`\cD=\cN(2.0,0.1)`, i.e.:

.. math::

    M_{f, \cD}(x) = \int_{Supp(\cD)} x \theta p(\theta) d \theta = x \mathbb{E}(\Theta)=2.0x 


2- Solution
-----------
.. math::

    M_{f, \cD}(1.0) = 2.0

3- Resolution
-------------
.. literalinclude:: ../../t_example5.py



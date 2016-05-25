Example 4
=========

1- Problem statement
--------------------
.. math::

    \begin{aligned}
    & \underset{x}{\text{minimize}}
    & & \mathbb{E}_{\cD}(\cos(x) \sin(\Theta)) \\
    & \text{subject to}
    & & \mathbb{P}_{\cD}(-2 - x + \Theta \leq 0) \geq 0.9 \\
    & & & x - 4 \leq 0 \\
    & & & \Theta \thicksim \cU(0, 2)
    \end{aligned}

2- Solution
-----------
.. math::

    x^* = \pi

3- Resolution
-------------
.. literalinclude:: ../../t_example4.py


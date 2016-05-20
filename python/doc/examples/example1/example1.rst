Example 1
=========

1- Problem statement
--------------------
.. math::

    \begin{aligned}
    & \underset{x}{\text{minimize}}
    & & \mathbb{E}_{\cD}((x_0-2)^2 + 2x_1^2 -4x_1 + \Theta) \\
    & \text{subject to}
    & & \mathbb{P}_{\cD}(-x_0 + 4x_1 + \Theta -3 \leq 0) \geq 0.9 \\
    & & & \Theta \thicksim \cU(1, 3)
    \end{aligned}

2- Solution
-----------
.. math::

    x^* = [2.2, 0.6]

3- Resolution
-------------
.. literalinclude:: ../../t_example1.py

Example 3
=========

1- Problem statement
--------------------
.. math::

    \begin{aligned}
    & \underset{x}{\text{minimize}}
    & & \mathbb{E}_{\cD}(x^3 - x + \Theta) \\
    & \text{subject to}
    & & \mathbb{P}_{\cD}(x + \Theta - 2 \leq 0) \geq 0.9 \\
    & & & \Theta \thicksim \cE(2)
    \end{aligned}

2- Solution
-----------
.. math::

    x^* = -1

3- Resolution
-------------
.. literalinclude:: ../../t_example3.py


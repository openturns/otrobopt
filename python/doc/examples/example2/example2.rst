Example 2
=========


1- Problem statement
--------------------
.. math::

    \begin{aligned}
    & \underset{x}{\text{minimize}}
    & & \mathbb{E}_{\cD}(\sqrt{x_0} \sqrt{x_1} \Theta) \\
    & \text{subject to}
    & & 2x_1 + 4x_0 - 120 \leq 0 \\
    & & & \Theta \thicksim \cN(1, 3)
    \end{aligned}

2- Solution
-----------
.. math::

    x^* = [15, 30]

3- Resolution
-------------
.. literalinclude:: ../../t_example2.py


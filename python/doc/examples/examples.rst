Examples
========

Example 1
---------

1- Problem statement
````````````````````
.. math::

    \begin{aligned}
    & \underset{x}{\text{minimize}}
    & & \mathbb{E}((x_0-2)^2 + 2x_1^2 -4x_1 + \theta) \\
    & \text{subject to}
    & & \mathbb{P}(-x_0 + 4x_1 + \theta -3 \leq 0) \geq 0.9 \\
    & & & \theta \thicksim \mathcal{U}([1, 3])
    \end{aligned}

Example 2
---------

1- Problem statement
````````````````````
.. math::

    \begin{aligned}
    & \underset{x}{\text{minimize}}
    & & \mathbb{E}(\sqrt{x_0 x_1} \theta) \\
    & \text{subject to}
    & & \mathbb{P}(2x_1 + 4x_0 - 120 \leq 0) \geq 0.9 \\
    & & & \theta \thicksim \mathcal{N}(1, 3)
    \end{aligned}

Example 3
---------

1- Problem statement
````````````````````
.. math::

    \begin{aligned}
    & \underset{x}{\text{minimize}}
    & & \mathbb{E}(x^3 - x + \theta) \\
    & \text{subject to}
    & & \mathbb{P}(x + \theta - 2\leq 0) \geq 0.9 \\
    & & & \theta \thicksim \epsilon(\lambda)
    \end{aligned}

Example 4
---------

1- Problem statement
````````````````````
.. math::

    \begin{aligned}
    & \underset{x}{\text{minimize}}
    & & \mathbb{E}(cos(x) sin(\theta)) \\
    & \text{subject to}
    & & \mathbb{P}(-2 - x + \theta \leq 0) \geq 0.9 \\
    & & & x - 4 \leq 0 \\
    & & & \theta \thicksim \mathcal{U}([0, 2])
    \end{aligned}

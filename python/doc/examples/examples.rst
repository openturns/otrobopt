Examples
========

Example 1
---------

1- Problem statement
````````````````````
.. math::

    \begin{aligned}
    & \underset{x}{\text{minimize}}
    & & \mathbb{E}_{\mathcal{D}}((x_0-2)^2 + 2x_1^2 -4x_1 + \Theta) \\
    & \text{subject to}
    & & \mathbb{P}_{\mathcal{D}}(-x_0 + 4x_1 + \Theta -3 \leq 0) \geq 0.9 \\
    & & & \Theta \thicksim \mathcal{U}(1, 3)
    \end{aligned}

2- Solution
```````````
.. math::

    x^* = [2.2, 0.6]

3- Resolution
`````````````
.. literalinclude:: ../t_example1.py

Example 2
---------

1- Problem statement
````````````````````
.. math::

    \begin{aligned}
    & \underset{x}{\text{minimize}}
    & & \mathbb{E}_{\mathcal{D}}(\sqrt{x_0} \sqrt{x_1} \Theta) \\
    & \text{subject to}
    & & \mathbb{P}_{\mathcal{D}}(2x_1 + 4x_0 - 120 \leq 0) \geq 0.9 \\
    & & & \Theta \thicksim \mathcal{N}(1, 3)
    \end{aligned}

2- Solution
```````````
.. math::

    x^* = [15, 30]

3- Resolution
`````````````
.. literalinclude:: ../t_example2.py

Example 3
---------

1- Problem statement
````````````````````
.. math::

    \begin{aligned}
    & \underset{x}{\text{minimize}}
    & & \mathbb{E}_{\mathcal{D}}(x^3 - x + \Theta) \\
    & \text{subject to}
    & & \mathbb{P}_{\mathcal{D}}(x + \Theta - 2 \leq 0) \geq 0.9 \\
    & & & \Theta \thicksim \cE(2)
    \end{aligned}

2- Solution
```````````
.. math::

    x^* = -1

3- Resolution
`````````````
.. literalinclude:: ../t_example3.py

Example 4
---------

1- Problem statement
````````````````````
.. math::

    \begin{aligned}
    & \underset{x}{\text{minimize}}
    & & \mathbb{E}_{\mathcal{D}}(\cos(x) \sin(\Theta)) \\
    & \text{subject to}
    & & \mathbb{P}_{\mathcal{D}}(-2 - x + \Theta \leq 0) \geq 0.9 \\
    & & & x - 4 \leq 0 \\
    & & & \Theta \thicksim \mathcal{U}(0, 2)
    \end{aligned}

2- Solution
```````````
.. math::

    x^* = \pi

3- Resolution
`````````````
.. literalinclude:: ../t_example4.py
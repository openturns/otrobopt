Validation
==========

Test case 1: XXXXXX
-------------------

The purpose of this example is to estimate the risk...

1- Problem statement
````````````````````

a- Inputs
'''''''''

- Stochastics variables:

====== ======================== ==================
 Name  Description              Distribution
====== ======================== ==================
Q      A1                       Normal(10200, 100)
E      A2                       Normal(3000, 15)
C      C1                       Normal(4000, 60)
====== ======================== ==================

- Deterministic independent variables:

======== ================================ =================
Variable Description                      Value
======== ================================ =================
t        Loss rate                        0.05
Nelec    Reference efficiency             0.54
Nth      Reference efficiency             0.8
======== ================================ =================

b- Output
'''''''''

Primary energy savings :math:`E_p`

.. math::

    B = 1-\frac{Q}{\frac{F}{(1-u)2}+\frac{L}{M}}



2- Deterministic parametric analysis
````````````````````````````````````

a- Inputs
'''''''''

======== ======= ======= ====
Variable Min     Max     Step
======== ======= ======= ====
Q        10033.1 10364.6 100
E        2975    3024.99 15
C        3901.33 4097.74 60
======== ======= ======= ====

b- Results
''''''''''

===== ==== ==== =======
Q     E    C    Ep 
===== ==== ==== =======
10030 2975 3900 0.06035
10365 2995 4100 0.05456
===== ==== ==== =======



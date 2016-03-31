Examples
========

Example 1: Axial stressed beam
------------------------------

This example is a simple beam, restrained at one side and stressed by a traction load F at the other side.

1- Problem statement
````````````````````

a- Inputs
'''''''''

- Stochastics variables:

====== ======================== ==================
 Name  Description              Distribution
====== ======================== ==================
F      Traction load            Normal(75e3, 5e3)
sigma  Axial stress             LogNormal(300, 30)
====== ======================== ==================

- Deterministic independent variables:

======== ================================ =================
Variable Description                      Value
======== ================================ =================
D        diameter                         20.0
======== ================================ =================

b- Output
'''''''''

Primary energy savings :math:`G`

.. math::

    G = \sigma_e -\frac{F}{\pi \frac{D^2}{4} }


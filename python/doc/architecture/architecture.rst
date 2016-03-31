Architecture considerations
===========================

Dependencies
------------

Several dependecies are needed in order to build the GUI:

 - OpenTURNS >= 1.5
 - Sphinx-doc (optional for this doc)

Compilation
-----------

.. code-block:: bash

    cd otgui
    mkdir -p build && cd build
    cmake \
      -DCMAKE_INSTALL_PREFIX=$PWD/install \
      -DOpenTURNS_DIR=$PWD/../../openturns-1.5/build/install/lib/cmake/openturns \
      ..

Source code structure
---------------------

Here is the global class diagram for each layer:

.. image:: class_diagram.png
    :align: center

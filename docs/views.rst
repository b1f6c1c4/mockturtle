Views
-----

Views can modify the implementation of a network interface by

1. adding interface methods that are not supported by the implementation,
2. changing the implementation of interface methods, and
3. deleting interface methods.

Views implement the network interface and can be passed like a network to an
algorithm.  Several views are implemented in mockturtle.

`topo_view`: Ensure topological order
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**Header:** ``mockturtle/views/topo_view.hpp``

.. doxygenclass:: mockturtle::topo_view
   :members:

`mapping_view`: Add mapping interface methods
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**Header:** ``mockturtle/views/mapping_view.hpp``

.. doxygenclass:: mockturtle::mapping_view
   :members:

`immutable_view`: Prevent network changes
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**Header:** ``mockturtle/views/immutable_view.hpp``

.. doxygenclass:: mockturtle::immutable_view
   :members:

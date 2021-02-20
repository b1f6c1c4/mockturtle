#include <catch.hpp>

#include <kitty/constructors.hpp>
#include <kitty/dynamic_truth_table.hpp>
#include <kitty/operations.hpp>
#include <kitty/operators.hpp>
#include <mockturtle/networks/dmig.hpp>
#include <mockturtle/traits.hpp>

using namespace mockturtle;

TEST_CASE( "create and use constants in an D-MIG", "[dmig]" )
{
  dmig_network dmig;

  CHECK( dmig.size() == 1 );
  CHECK( has_get_constant_v<dmig_network> );
  CHECK( has_is_constant_v<dmig_network> );
  CHECK( has_get_node_v<dmig_network> );
  CHECK( has_is_complemented_v<dmig_network> );

  const auto c0 = dmig.get_constant( false );
  CHECK( dmig.is_constant( dmig.get_node( c0 ) ) );
  CHECK( !dmig.is_pi( dmig.get_node( c0 ) ) );

  CHECK( dmig.size() == 1 );
  CHECK( std::is_same_v<std::decay_t<decltype( c0 )>, dmig_network::signal> );
  CHECK( dmig.get_node( c0 ) == 0 );
  CHECK( !dmig.is_complemented( c0 ) );

  const auto c1 = dmig.get_constant( true );

  CHECK( dmig.get_node( c1 ) == 0 );
  CHECK( dmig.is_complemented( c1 ) );

  CHECK( c0 != c1 );
  CHECK( c0 == !c1 );
  CHECK( ( !c0 ) == c1 );
  CHECK( ( !c0 ) != !c1 );
  CHECK( -c0 == c1 );
  CHECK( -c1 == c1 );
  CHECK( c0 == +c1 );
  CHECK( c0 == +c0 );
}

TEST_CASE( "create and use primary inputs in an D-MIG", "[dmig]" )
{
  dmig_network dmig;

  CHECK( has_create_pi_v<dmig_network> );

  auto a = dmig.create_pi();

  CHECK( dmig.size() == 2 );
  CHECK( dmig.num_pis() == 1 );
  CHECK( dmig.num_gates() == 0 );

  CHECK( std::is_same_v<std::decay_t<decltype( a )>, dmig_network::signal> );

  CHECK( a.index == 1 );
  CHECK( a.complement == 0 );

  a = !a;

  CHECK( a.index == 1 );
  CHECK( a.complement == 1 );

  a = +a;

  CHECK( a.index == 1 );
  CHECK( a.complement == 0 );

  a = +a;

  CHECK( a.index == 1 );
  CHECK( a.complement == 0 );

  a = -a;

  CHECK( a.index == 1 );
  CHECK( a.complement == 1 );

  a = -a;

  CHECK( a.index == 1 );
  CHECK( a.complement == 1 );

  a = a ^ true;

  CHECK( a.index == 1 );
  CHECK( a.complement == 0 );

  a = a ^ true;

  CHECK( a.index == 1 );
  CHECK( a.complement == 1 );
}

TEST_CASE( "create and use primary outputs in an D-MIG", "[dmig]" )
{
  dmig_network dmig;

  CHECK( has_create_po_v<dmig_network> );

  const auto c0 = dmig.get_constant( false );
  const auto x1 = dmig.create_pi();

  CHECK( dmig.size() == 2 );
  CHECK( dmig.num_pis() == 1 );
  CHECK( dmig.num_pos() == 0 );

  dmig.create_po( c0 );
  dmig.create_po( x1 );
  dmig.create_po( !x1 );

  CHECK( dmig.size() == 2 );
  CHECK( dmig.num_pos() == 3 );

  dmig.foreach_po( [&]( auto s, auto i ) {
    switch ( i )
    {
    case 0:
      CHECK( s == c0 );
      break;
    case 1:
      CHECK( s == x1 );
      break;
    case 2:
      CHECK( s == !x1 );
      break;
    }
  } );
}

TEST_CASE( "create and use register in an D-MIG", "[dmig]" )
{
  dmig_network dmig;

  CHECK( has_foreach_po_v<dmig_network> );
  CHECK( has_create_po_v<dmig_network> );
  CHECK( has_create_pi_v<dmig_network> );
  CHECK( has_create_ro_v<dmig_network> );
  CHECK( has_create_ri_v<dmig_network> );
  CHECK( has_create_maj_v<dmig_network> );

  const auto c0 = dmig.get_constant( false );
  const auto x1 = dmig.create_pi();
  const auto x2 = dmig.create_pi();
  const auto x3 = dmig.create_pi();
  const auto x4 = dmig.create_pi();

  CHECK( dmig.size() == 5 );
  CHECK( dmig.num_registers() == 0 );
  CHECK( dmig.num_pis() == 4 );
  CHECK( dmig.num_pos() == 0 );
  CHECK( dmig.is_combinational() );

  const auto f1 = dmig.create_maj( x1, x2, x3 );
  dmig.create_po( f1 );
  dmig.create_po( !f1 );

  const auto f2 = dmig.create_maj( f1, x4, c0 );
  dmig.create_ri( f2 );

  const auto ro = dmig.create_ro();
  dmig.create_po( ro );

  CHECK( dmig.num_pos() == 3 );
  CHECK( dmig.num_registers() == 1 );
  CHECK( !dmig.is_combinational() );

  dmig.foreach_po( [&]( auto s, auto i ) {
    switch ( i )
    {
    case 0:
      CHECK( s == f1 );
      break;
    case 1:
      CHECK( s == !f1 );
      break;
    case 2:
      // Check if the output (connected to the register) data is the same as the node data being registered.
      CHECK( f2.data == dmig.po_at( i ).data );
      break;
    default:
      CHECK( false );
      break;
    }
  } );
}

TEST_CASE( "create unary operations in an D-MIG", "[dmig]" )
{
  dmig_network dmig;

  CHECK( has_create_buf_v<dmig_network> );
  CHECK( has_create_not_v<dmig_network> );

  auto x1 = dmig.create_pi();

  CHECK( dmig.size() == 2 );

  auto f1 = dmig.create_buf( x1 );
  auto f2 = dmig.create_not( x1 );

  CHECK( dmig.size() == 2 );
  CHECK( f1 == x1 );
  CHECK( f2 == !x1 );
}

TEST_CASE( "create binary and ternary operations in an D-MIG", "[dmig]" )
{
  dmig_network dmig;

  CHECK( has_create_and_v<dmig_network> );
  CHECK( has_create_nand_v<dmig_network> );
  CHECK( has_create_or_v<dmig_network> );
  CHECK( has_create_nor_v<dmig_network> );
  CHECK( has_create_xor_v<dmig_network> );
  CHECK( has_create_maj_v<dmig_network> );

  const auto x1 = dmig.create_pi();
  const auto x2 = dmig.create_pi();

  CHECK( dmig.size() == 3 );

  const auto f1 = dmig.create_and( x1, x2 );
  CHECK( dmig.size() == 4 );
  CHECK( dmig.num_gates() == 1 );

  const auto f2 = dmig.create_nand( x1, x2 );
  CHECK( dmig.size() == 4 );
  CHECK( f1 == !f2 );

  const auto f3 = dmig.create_or( x1, x2 );
  CHECK( dmig.size() == 5 );

  const auto f4 = dmig.create_nor( x1, x2 );
  CHECK( dmig.size() == 5 );
  CHECK( f3 == !f4 );

  dmig.create_xor( x1, x2 );
  CHECK( dmig.size() == 8 );

  dmig.create_maj( x1, x2, f1 );
  CHECK( dmig.size() == 9 );

  const auto f6 = dmig.create_maj( x1, x2, dmig.get_constant( false ) );
  CHECK( dmig.size() == 9 );
  CHECK( f1 == f6 );

  const auto f7 = dmig.create_maj( x1, x2, dmig.get_constant( true ) );
  CHECK( dmig.size() == 9 );
  CHECK( f3 == f7 );

  const auto x3 = dmig.create_pi();

  const auto f8 = dmig.create_maj( x1, x2, x3 );
  const auto f9 = dmig.create_maj( !x1, !x2, !x3 );
  CHECK( f8 == !f9 );
}

TEST_CASE( "hash nodes in D-MIG network", "[dmig]" )
{
  dmig_network dmig;

  auto a = dmig.create_pi();
  auto b = dmig.create_pi();
  auto c = dmig.create_pi();

  auto f = dmig.create_maj( a, b, c );
  auto g = dmig.create_maj( a, b, c );

  CHECK( dmig.size() == 5u );
  CHECK( dmig.num_gates() == 1u );

  CHECK( dmig.get_node( f ) == dmig.get_node( g ) );

  auto f1 = dmig.create_maj( a, !b, c );
  auto g1 = dmig.create_maj( a, !b, c );

  CHECK( dmig.size() == 6u );
  CHECK( dmig.num_gates() == 2u );

  CHECK( dmig.get_node( f1 ) == dmig.get_node( g1 ) );
}

TEST_CASE( "clone a node in D-MIG network", "[dmig]" )
{
  dmig_network dmig1, dmig2;

  CHECK( has_clone_node_v<dmig_network> );

  auto a1 = dmig1.create_pi();
  auto b1 = dmig1.create_pi();
  auto c1 = dmig1.create_pi();
  auto f1 = dmig1.create_maj( a1, b1, c1 );
  CHECK( dmig1.size() == 5 );

  auto a2 = dmig2.create_pi();
  auto b2 = dmig2.create_pi();
  auto c2 = dmig2.create_pi();
  CHECK( dmig2.size() == 4 );

  auto f2 = dmig2.clone_node( dmig1, dmig1.get_node( f1 ), {a2, b2, c2} );
  CHECK( dmig2.size() == 5 );

  dmig2.foreach_fanin( dmig2.get_node( f2 ), [&]( auto const& s, auto ) {
    CHECK( !dmig2.is_complemented( s ) );
  } );
}

TEST_CASE( "structural properties of an D-MIG", "[dmig]" )
{
  dmig_network dmig;

  CHECK( has_size_v<dmig_network> );
  CHECK( has_num_pis_v<dmig_network> );
  CHECK( has_num_pos_v<dmig_network> );
  CHECK( has_num_gates_v<dmig_network> );
  CHECK( has_fanin_size_v<dmig_network> );
  CHECK( has_fanout_size_v<dmig_network> );

  const auto x1 = dmig.create_pi();
  const auto x2 = dmig.create_pi();
  const auto x3 = dmig.create_pi();

  const auto f1 = dmig.create_maj( x1, x2, x3 );
  const auto f2 = dmig.create_maj( x1, x2, !x3 );

  dmig.create_po( f1 );
  dmig.create_po( f2 );

  CHECK( dmig.size() == 6 );
  CHECK( dmig.num_pis() == 3 );
  CHECK( dmig.num_pos() == 2 );
  CHECK( dmig.num_gates() == 2 );
  CHECK( dmig.fanin_size( dmig.get_node( x1 ) ) == 0 );
  CHECK( dmig.fanin_size( dmig.get_node( x2 ) ) == 0 );
  CHECK( dmig.fanin_size( dmig.get_node( x3 ) ) == 0 );
  CHECK( dmig.fanin_size( dmig.get_node( f1 ) ) == 3 );
  CHECK( dmig.fanin_size( dmig.get_node( f2 ) ) == 3 );
  CHECK( dmig.fanout_size( dmig.get_node( x1 ) ) == 2 );
  CHECK( dmig.fanout_size( dmig.get_node( x2 ) ) == 2 );
  CHECK( dmig.fanout_size( dmig.get_node( f1 ) ) == 1 );
  CHECK( dmig.fanout_size( dmig.get_node( f2 ) ) == 1 );
}

TEST_CASE( "node and signal iteration in an D-MIG", "[dmig]" )
{
  dmig_network dmig;

  CHECK( has_foreach_node_v<dmig_network> );
  CHECK( has_foreach_pi_v<dmig_network> );
  CHECK( has_foreach_po_v<dmig_network> );
  CHECK( has_foreach_gate_v<dmig_network> );
  CHECK( has_foreach_fanin_v<dmig_network> );

  const auto x1 = dmig.create_pi();
  const auto x2 = dmig.create_pi();
  const auto x3 = dmig.create_pi();
  const auto f1 = dmig.create_maj( x1, x2, x3 );
  const auto f2 = dmig.create_maj( x1, x2, !x3 );
  dmig.create_po( f1 );
  dmig.create_po( f2 );

  CHECK( dmig.size() == 6 );

  /* iterate over nodes */
  uint32_t mask{0}, counter{0};
  dmig.foreach_node( [&]( auto n, auto i ) { mask |= ( 1 << n ); counter += i; } );
  CHECK( mask == 63 );
  CHECK( counter == 15 );

  mask = 0;
  dmig.foreach_node( [&]( auto n ) { mask |= ( 1 << n ); } );
  CHECK( mask == 63 );

  mask = counter = 0;
  dmig.foreach_node( [&]( auto n, auto i ) { mask |= ( 1 << n ); counter += i; return false; } );
  CHECK( mask == 1 );
  CHECK( counter == 0 );

  mask = 0;
  dmig.foreach_node( [&]( auto n ) { mask |= ( 1 << n ); return false; } );
  CHECK( mask == 1 );

  /* iterate over PIs */
  mask = counter = 0;
  dmig.foreach_pi( [&]( auto n, auto i ) { mask |= ( 1 << n ); counter += i; } );
  CHECK( mask == 14 );
  CHECK( counter == 3 );

  mask = 0;
  dmig.foreach_pi( [&]( auto n ) { mask |= ( 1 << n ); } );
  CHECK( mask == 14 );

  mask = counter = 0;
  dmig.foreach_pi( [&]( auto n, auto i ) { mask |= ( 1 << n ); counter += i; return false; } );
  CHECK( mask == 2 );
  CHECK( counter == 0 );

  mask = 0;
  dmig.foreach_pi( [&]( auto n ) { mask |= ( 1 << n ); return false; } );
  CHECK( mask == 2 );

  /* iterate over POs */
  mask = counter = 0;
  dmig.foreach_po( [&]( auto s, auto i ) { mask |= ( 1 << dmig.get_node( s ) ); counter += i; } );
  CHECK( mask == 48 );
  CHECK( counter == 1 );

  mask = 0;
  dmig.foreach_po( [&]( auto s ) { mask |= ( 1 << dmig.get_node( s ) ); } );
  CHECK( mask == 48 );

  mask = counter = 0;
  dmig.foreach_po( [&]( auto s, auto i ) { mask |= ( 1 << dmig.get_node( s ) ); counter += i; return false; } );
  CHECK( mask == 16 );
  CHECK( counter == 0 );

  mask = 0;
  dmig.foreach_po( [&]( auto s ) { mask |= ( 1 << dmig.get_node( s ) ); return false; } );
  CHECK( mask == 16 );

  /* iterate over gates */
  mask = counter = 0;
  dmig.foreach_gate( [&]( auto n, auto i ) { mask |= ( 1 << n ); counter += i; } );
  CHECK( mask == 48 );
  CHECK( counter == 1 );

  mask = 0;
  dmig.foreach_gate( [&]( auto n ) { mask |= ( 1 << n ); } );
  CHECK( mask == 48 );

  mask = counter = 0;
  dmig.foreach_gate( [&]( auto n, auto i ) { mask |= ( 1 << n ); counter += i; return false; } );
  CHECK( mask == 16 );
  CHECK( counter == 0 );

  mask = 0;
  dmig.foreach_gate( [&]( auto n ) { mask |= ( 1 << n ); return false; } );
  CHECK( mask == 16 );

  /* iterate over fanins */
  mask = counter = 0;
  dmig.foreach_fanin( dmig.get_node( f1 ), [&]( auto s, auto i ) { mask |= ( 1 << dmig.get_node( s ) ); counter += i; } );
  CHECK( mask == 14 );
  CHECK( counter == 3 );

  mask = 0;
  dmig.foreach_fanin( dmig.get_node( f1 ), [&]( auto s ) { mask |= ( 1 << dmig.get_node( s ) ); } );
  CHECK( mask == 14 );

  mask = counter = 0;
  dmig.foreach_fanin( dmig.get_node( f1 ), [&]( auto s, auto i ) { mask |= ( 1 << dmig.get_node( s ) ); counter += i; return false; } );
  CHECK( mask == 2 );
  CHECK( counter == 0 );

  mask = 0;
  dmig.foreach_fanin( dmig.get_node( f1 ), [&]( auto s ) { mask |= ( 1 << dmig.get_node( s ) ); return false; } );
  CHECK( mask == 2 );
}

TEST_CASE( "compute values in D-MIGs", "[dmig]" )
{
  dmig_network dmig;

  CHECK( has_compute_v<dmig_network, bool> );
  CHECK( has_compute_v<dmig_network, kitty::dynamic_truth_table> );
  CHECK( has_compute_v<dmig_network, kitty::partial_truth_table> );
  CHECK( has_compute_inplace_v<dmig_network, kitty::partial_truth_table> );

  const auto x1 = dmig.create_pi();
  const auto x2 = dmig.create_pi();
  const auto x3 = dmig.create_pi();
  const auto f1 = dmig.create_maj( !x1, x2, x3 );
  const auto f2 = dmig.create_maj( x1, !x2, x3 );
  dmig.create_po( f1 );
  dmig.create_po( f2 );

  {
    std::vector<bool> values{{true, false, true}};

    CHECK( dmig.compute( dmig.get_node( f1 ), values.begin(), values.end() ) == false );
    CHECK( dmig.compute( dmig.get_node( f2 ), values.begin(), values.end() ) == true );
  }

  {
    std::vector<kitty::dynamic_truth_table> xs{3, kitty::dynamic_truth_table( 3 )};
    kitty::create_nth_var( xs[0], 0 );
    kitty::create_nth_var( xs[1], 1 );
    kitty::create_nth_var( xs[2], 2 );

    CHECK( dmig.compute( dmig.get_node( f1 ), xs.begin(), xs.end() ) == ( ( ~xs[0] & xs[1] ) | ( ~xs[0] & xs[2] ) | ( xs[2] & xs[1] ) ) );
    CHECK( dmig.compute( dmig.get_node( f2 ), xs.begin(), xs.end() ) == ( ( xs[0] & ~xs[1] ) | ( xs[0] & xs[2] ) | ( xs[2] & ~xs[1] ) ) );
  }

  {
    std::vector<kitty::partial_truth_table> xs{3};

    CHECK( dmig.compute( dmig.get_node( f1 ), xs.begin(), xs.end() ) == ( ( ~xs[0] & xs[1] ) | ( ~xs[0] & xs[2] ) | ( xs[2] & xs[1] ) ) );
    CHECK( dmig.compute( dmig.get_node( f2 ), xs.begin(), xs.end() ) == ( ( xs[0] & ~xs[1] ) | ( xs[0] & xs[2] ) | ( xs[2] & ~xs[1] ) ) );

    xs[0].add_bit( 0 ); xs[1].add_bit( 0 ); xs[2].add_bit( 0 );

    CHECK( dmig.compute( dmig.get_node( f1 ), xs.begin(), xs.end() ) == ( ( ~xs[0] & xs[1] ) | ( ~xs[0] & xs[2] ) | ( xs[2] & xs[1] ) ) );
    CHECK( dmig.compute( dmig.get_node( f2 ), xs.begin(), xs.end() ) == ( ( xs[0] & ~xs[1] ) | ( xs[0] & xs[2] ) | ( xs[2] & ~xs[1] ) ) );

    xs[0].add_bit( 0 ); xs[1].add_bit( 0 ); xs[2].add_bit( 1 );

    CHECK( dmig.compute( dmig.get_node( f1 ), xs.begin(), xs.end() ) == ( ( ~xs[0] & xs[1] ) | ( ~xs[0] & xs[2] ) | ( xs[2] & xs[1] ) ) );
    CHECK( dmig.compute( dmig.get_node( f2 ), xs.begin(), xs.end() ) == ( ( xs[0] & ~xs[1] ) | ( xs[0] & xs[2] ) | ( xs[2] & ~xs[1] ) ) );

    xs[0].add_bit( 0 ); xs[1].add_bit( 1 ); xs[2].add_bit( 0 );

    CHECK( dmig.compute( dmig.get_node( f1 ), xs.begin(), xs.end() ) == ( ( ~xs[0] & xs[1] ) | ( ~xs[0] & xs[2] ) | ( xs[2] & xs[1] ) ) );
    CHECK( dmig.compute( dmig.get_node( f2 ), xs.begin(), xs.end() ) == ( ( xs[0] & ~xs[1] ) | ( xs[0] & xs[2] ) | ( xs[2] & ~xs[1] ) ) );

    xs[0].add_bit( 0 ); xs[1].add_bit( 1 ); xs[2].add_bit( 1 );

    CHECK( dmig.compute( dmig.get_node( f1 ), xs.begin(), xs.end() ) == ( ( ~xs[0] & xs[1] ) | ( ~xs[0] & xs[2] ) | ( xs[2] & xs[1] ) ) );
    CHECK( dmig.compute( dmig.get_node( f2 ), xs.begin(), xs.end() ) == ( ( xs[0] & ~xs[1] ) | ( xs[0] & xs[2] ) | ( xs[2] & ~xs[1] ) ) );

    xs[0].add_bit( 1 ); xs[1].add_bit( 0 ); xs[2].add_bit( 0 );

    CHECK( dmig.compute( dmig.get_node( f1 ), xs.begin(), xs.end() ) == ( ( ~xs[0] & xs[1] ) | ( ~xs[0] & xs[2] ) | ( xs[2] & xs[1] ) ) );
    CHECK( dmig.compute( dmig.get_node( f2 ), xs.begin(), xs.end() ) == ( ( xs[0] & ~xs[1] ) | ( xs[0] & xs[2] ) | ( xs[2] & ~xs[1] ) ) );

    xs[0].add_bit( 1 ); xs[1].add_bit( 0 ); xs[2].add_bit( 1 );

    CHECK( dmig.compute( dmig.get_node( f1 ), xs.begin(), xs.end() ) == ( ( ~xs[0] & xs[1] ) | ( ~xs[0] & xs[2] ) | ( xs[2] & xs[1] ) ) );
    CHECK( dmig.compute( dmig.get_node( f2 ), xs.begin(), xs.end() ) == ( ( xs[0] & ~xs[1] ) | ( xs[0] & xs[2] ) | ( xs[2] & ~xs[1] ) ) );

    xs[0].add_bit( 1 ); xs[1].add_bit( 1 ); xs[2].add_bit( 0 );

    CHECK( dmig.compute( dmig.get_node( f1 ), xs.begin(), xs.end() ) == ( ( ~xs[0] & xs[1] ) | ( ~xs[0] & xs[2] ) | ( xs[2] & xs[1] ) ) );
    CHECK( dmig.compute( dmig.get_node( f2 ), xs.begin(), xs.end() ) == ( ( xs[0] & ~xs[1] ) | ( xs[0] & xs[2] ) | ( xs[2] & ~xs[1] ) ) );

    xs[0].add_bit( 1 ); xs[1].add_bit( 1 ); xs[2].add_bit( 1 );

    CHECK( dmig.compute( dmig.get_node( f1 ), xs.begin(), xs.end() ) == ( ( ~xs[0] & xs[1] ) | ( ~xs[0] & xs[2] ) | ( xs[2] & xs[1] ) ) );
    CHECK( dmig.compute( dmig.get_node( f2 ), xs.begin(), xs.end() ) == ( ( xs[0] & ~xs[1] ) | ( xs[0] & xs[2] ) | ( xs[2] & ~xs[1] ) ) );
  }

  {
    std::vector<kitty::partial_truth_table> xs{3};
    kitty::partial_truth_table result;

    xs[0].add_bit( 0 ); xs[1].add_bit( 0 ); xs[2].add_bit( 0 );

    dmig.compute( dmig.get_node( f1 ), result, xs.begin(), xs.end() ); CHECK( result == ( ( ~xs[0] & xs[1] ) | ( ~xs[0] & xs[2] ) | ( xs[2] & xs[1] ) ) );
    dmig.compute( dmig.get_node( f2 ), result, xs.begin(), xs.end() ); CHECK( result == ( ( xs[0] & ~xs[1] ) | ( xs[0] & xs[2] ) | ( xs[2] & ~xs[1] ) ) );

    xs[0].add_bit( 0 ); xs[1].add_bit( 0 ); xs[2].add_bit( 1 );

    dmig.compute( dmig.get_node( f1 ), result, xs.begin(), xs.end() ); CHECK( result == ( ( ~xs[0] & xs[1] ) | ( ~xs[0] & xs[2] ) | ( xs[2] & xs[1] ) ) );
    dmig.compute( dmig.get_node( f2 ), result, xs.begin(), xs.end() ); CHECK( result == ( ( xs[0] & ~xs[1] ) | ( xs[0] & xs[2] ) | ( xs[2] & ~xs[1] ) ) );

    xs[0].add_bit( 0 ); xs[1].add_bit( 1 ); xs[2].add_bit( 0 );

    dmig.compute( dmig.get_node( f1 ), result, xs.begin(), xs.end() ); CHECK( result == ( ( ~xs[0] & xs[1] ) | ( ~xs[0] & xs[2] ) | ( xs[2] & xs[1] ) ) );
    dmig.compute( dmig.get_node( f2 ), result, xs.begin(), xs.end() ); CHECK( result == ( ( xs[0] & ~xs[1] ) | ( xs[0] & xs[2] ) | ( xs[2] & ~xs[1] ) ) );

    xs[0].add_bit( 0 ); xs[1].add_bit( 1 ); xs[2].add_bit( 1 );

    dmig.compute( dmig.get_node( f1 ), result, xs.begin(), xs.end() ); CHECK( result == ( ( ~xs[0] & xs[1] ) | ( ~xs[0] & xs[2] ) | ( xs[2] & xs[1] ) ) );
    dmig.compute( dmig.get_node( f2 ), result, xs.begin(), xs.end() ); CHECK( result == ( ( xs[0] & ~xs[1] ) | ( xs[0] & xs[2] ) | ( xs[2] & ~xs[1] ) ) );

    xs[0].add_bit( 1 ); xs[1].add_bit( 0 ); xs[2].add_bit( 0 );

    dmig.compute( dmig.get_node( f1 ), result, xs.begin(), xs.end() ); CHECK( result == ( ( ~xs[0] & xs[1] ) | ( ~xs[0] & xs[2] ) | ( xs[2] & xs[1] ) ) );
    dmig.compute( dmig.get_node( f2 ), result, xs.begin(), xs.end() ); CHECK( result == ( ( xs[0] & ~xs[1] ) | ( xs[0] & xs[2] ) | ( xs[2] & ~xs[1] ) ) );

    dmig.compute( dmig.get_node( f1 ), result, xs.begin(), xs.end() ); CHECK( result == ( ( ~xs[0] & xs[1] ) | ( ~xs[0] & xs[2] ) | ( xs[2] & xs[1] ) ) );
    dmig.compute( dmig.get_node( f2 ), result, xs.begin(), xs.end() ); CHECK( result == ( ( xs[0] & ~xs[1] ) | ( xs[0] & xs[2] ) | ( xs[2] & ~xs[1] ) ) );

    xs[0].add_bit( 1 ); xs[1].add_bit( 0 ); xs[2].add_bit( 1 );

    dmig.compute( dmig.get_node( f1 ), result, xs.begin(), xs.end() ); CHECK( result == ( ( ~xs[0] & xs[1] ) | ( ~xs[0] & xs[2] ) | ( xs[2] & xs[1] ) ) );
    dmig.compute( dmig.get_node( f2 ), result, xs.begin(), xs.end() ); CHECK( result == ( ( xs[0] & ~xs[1] ) | ( xs[0] & xs[2] ) | ( xs[2] & ~xs[1] ) ) );

    xs[0].add_bit( 1 ); xs[1].add_bit( 1 ); xs[2].add_bit( 0 );

    dmig.compute( dmig.get_node( f1 ), result, xs.begin(), xs.end() ); CHECK( result == ( ( ~xs[0] & xs[1] ) | ( ~xs[0] & xs[2] ) | ( xs[2] & xs[1] ) ) );
    dmig.compute( dmig.get_node( f2 ), result, xs.begin(), xs.end() ); CHECK( result == ( ( xs[0] & ~xs[1] ) | ( xs[0] & xs[2] ) | ( xs[2] & ~xs[1] ) ) );

    xs[0].add_bit( 1 ); xs[1].add_bit( 1 ); xs[2].add_bit( 1 );

    dmig.compute( dmig.get_node( f1 ), result, xs.begin(), xs.end() ); CHECK( result == ( ( ~xs[0] & xs[1] ) | ( ~xs[0] & xs[2] ) | ( xs[2] & xs[1] ) ) );
    dmig.compute( dmig.get_node( f2 ), result, xs.begin(), xs.end() ); CHECK( result == ( ( xs[0] & ~xs[1] ) | ( xs[0] & xs[2] ) | ( xs[2] & ~xs[1] ) ) );
  }
}

TEST_CASE( "custom node values in D-MIGs", "[dmig]" )
{
  dmig_network dmig;

  CHECK( has_clear_values_v<dmig_network> );
  CHECK( has_value_v<dmig_network> );
  CHECK( has_set_value_v<dmig_network> );
  CHECK( has_incr_value_v<dmig_network> );
  CHECK( has_decr_value_v<dmig_network> );

  const auto x1 = dmig.create_pi();
  const auto x2 = dmig.create_pi();
  const auto x3 = dmig.create_pi();
  const auto f1 = dmig.create_maj( x1, x2, x3 );
  const auto f2 = dmig.create_maj( !x1, x2, x3 );
  dmig.create_po( f1 );
  dmig.create_po( f2 );

  CHECK( dmig.size() == 6 );

  dmig.clear_values();
  dmig.foreach_node( [&]( auto n ) {
    CHECK( dmig.value( n ) == 0 );
    dmig.set_value( n, static_cast<uint32_t>( n ) );
    CHECK( dmig.value( n ) == n );
    CHECK( dmig.incr_value( n ) == n );
    CHECK( dmig.value( n ) == n + 1 );
    CHECK( dmig.decr_value( n ) == n );
    CHECK( dmig.value( n ) == n );
  } );
  dmig.clear_values();
  dmig.foreach_node( [&]( auto n ) {
    CHECK( dmig.value( n ) == 0 );
  } );
}

TEST_CASE( "visited values in D-MIGs", "[dmig]" )
{
  dmig_network dmig;

  CHECK( has_clear_visited_v<dmig_network> );
  CHECK( has_visited_v<dmig_network> );
  CHECK( has_set_visited_v<dmig_network> );

  const auto x1 = dmig.create_pi();
  const auto x2 = dmig.create_pi();
  const auto x3 = dmig.create_pi();
  const auto f1 = dmig.create_maj( x1, x2, x3 );
  const auto f2 = dmig.create_and( x1, x2 );
  dmig.create_po( f1 );
  dmig.create_po( f2 );

  CHECK( dmig.size() == 6 );

  dmig.clear_visited();
  dmig.foreach_node( [&]( auto n ) {
    CHECK( dmig.visited( n ) == 0 );
    dmig.set_visited( n, static_cast<uint32_t>( n ) );
    CHECK( dmig.visited( n ) == n );
  } );
  dmig.clear_visited();
  dmig.foreach_node( [&]( auto n ) {
    CHECK( dmig.visited( n ) == 0 );
  } );
}

TEST_CASE( "node substitution in D-MIGs", "[dmig]" )
{
  dmig_network dmig;
  const auto a = dmig.create_pi();
  const auto b = dmig.create_pi();
  const auto f = dmig.create_and( a, b );

  CHECK( dmig.size() == 4 );

  dmig.foreach_fanin( dmig.get_node( f ), [&]( auto const& s ) {
    CHECK( !dmig.is_complemented( s ) );
  } );

  dmig.substitute_node( dmig.get_node( dmig.get_constant( false ) ), dmig.get_constant( true ) );

  CHECK( dmig.size() == 4 );

  dmig.foreach_fanin( dmig.get_node( f ), [&]( auto const& s, auto i ) {
    switch ( i )
    {
    case 0:
      CHECK( dmig.is_complemented( s ) );
      break;
    default:
      CHECK( !dmig.is_complemented( s ) );
      break;
    }
  } );
}

Chunky Dungeon Digger Library
=============================

This is a quick and dirty library for digging mines and man-made
underground tunnels for use in games such as roguelikes. The
dungeons are deterministically generated in areas called "chunks"
where each chunk can have one exit to each side, and neighbouring
chunks will get matching exits.

It can pseudo-randomly place doors and one-way doors in the
dungeon and attempt to place them to minimize backtracking.

Each room contains an isolatedness score which tells you how far
it is off the main path through the chunk.

Example dungeon:
```
   #####################+##################################
   #............#......#.#..........#........#............#
   #............#......#.#..........[........#............#
   #............#......#.#..........#........+............#
   #............#......#.####+#######........#............#
   #............+......#.#.........##........############^#######
   #............#......+.#.........###+#####^#####.....#........#
   #............#......#.#.........#......#......#.....#........#
   #............#......#.#.........#......#......#.....#........#
   #............#......#.#.........#......#......#.....#........#
   #............#......#.#.........#......[......#.....+........#
   ########+############.#.........#......#......#.....#........#
     #.......#........##.#.........#......###+####.....#........#
     #.......]........##.#.........#......#......+.....###+######
     #.......#........##.+.........#......#......#.....[........#
 ######_#########+######.#.........#......#......#.....#........#
 +.......................#.........+......#......#.....#........#
 ########+#####^########.#.........#......+......###^####_#######
    #.........#...#    #.#.........#......#......# #...........# 
    #.........+...######.#.........#......#......# #...........# 
    #.........#...]..###.#.........############+####...........# 
    #.........#####..###.#.........#     #.........#...........# 
    #.........[...#..###.#################.........+...........# 
    #.........#...#..###.#  #............[.........#...........# 
 ######+#######...#..###.#  #............###########...........# 
 #...........##...#..###.#  #............#         #...........# 
 #...........##...#....+.#########+#########################_####
 #...........##...+..###........................................+
 #...........##...######.########################################
 #...........##...#    #.#
 #...........##...#    #.#
 ##################    #+#
```

This could be generated with just this code:
```c++
chunkconfig config(s);
chunk c(config); // create with default size and location
c.generate_exits(); // create our exits depending our chunk location
chunk_filter_connect_exits(c); // make sure we have conenctivity
chunk_filter_room_expand(c); // this is where most rooms are made
chunk_filter_one_way_doors(c); // add secret one-way doors
c.beautify(); // prepare it for pretty printing
c.print_chunk(); // print to screen
```

Chunky will generate a basic map using your chosen configuration,
then you can use its filters (or your own filters) to improve
upon this map chunk.

It contains a list of all the rooms with some basic data like
location, size and where the exits are.

Runner
------

There is a very simple ncurses-based rogue-like in the `runner` directory
called (you guessed it) *runner*. It basically just allows you to run
around in chunky-generated maps to look at them. Press arrow keys to move
and 'esc' or 'q' to exit.

Building
========

For Ubuntu x86/x64, install these packages:

	sudo apt-get install git cmake pkg-config libncurses-dev

To build for linux desktop:
--------------------------

```
git submodule update --init --recursive
mkdir build
cd build
cmake ..
make
```

Then in order to verify that everything is working correctly:

```
make test
```

and then run

```
./chunkgen
```

as many times as you want and gawk at the pretty maps it spits out! Or just run

```
./runner
```

to pretend to play a rogue-like with our chunky maps.

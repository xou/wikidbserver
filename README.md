# dbpedia-server - Wikipedia Graph queries from ~3GB RAM

When importing wikipedia graph data to a graph database, such as neo4j, the import process
typically requires multiple tens of gigabytes of RAM and/or multiple hours of processing time.

dbpedia-server is capable of performing a complete import of all page labels (names) and links
in a few minutes. It keeps the entire database in typically 2.3-4GB ram (see performance characteristics
below).

## Building &amp; Running

- Download the labels and links dataset for a specific language from
  dbpedia (see "Data input" below).
- Build using "make". Requires boost, libbz2 and a C++11 capable compiler.
- Launch using `./wikidbserver --labels <labels.bz2> [--links <links.bz2>] [--inlinks]`
- Tests can be found in the ./test/ subdirectory, run them with `make test`. Requires googletest and googlemock.

## Command set

Currently, a basic set of commands is supported through a CLI interface:
```
 resource <resource>
   -- find an entry by resource (Usually Wikipedia URL)
 label <label>
   -- find an entry by page title ("label")
 id <id>
   -- describe the entry by a specific id
 outs <id>
   -- show all outgoing page links of page with the given id
 ins <id>
   -- show all incoming page links of page with given id (only
      available if invoked with --inlinks)
 inouts <id>
   -- show both in- and outgoing links.
 path[*] <from_id> <to_id>
   -- path: find a path between two pages, using only outgoing links, using BFS.
   -- path*: find many paths.
 path-undirected[*] <from_id> <to_id>
   -- find a path between two pages, using both incoming and outgoing nodes.
 path-exclude-add <id>
   -- add a page ID which should be excluded for graph queries
 path-exclude-clear
   -- clear the set of page IDs that should be excluded
```

## Example session:

Find and inspect data:

```
> resource Coffee         // query by resource (Wikipedia URL part)
  2391008 : Coffee "Coffee"
[0s]
> label Graph theory      // query by page title
  4180390 : Graph_theory "Graph theory"
[0s]
> id 744284               // look up ID
   744284 : Algorithm "Algorithm"
[0s]
```

Play [6 Degrees of Wikipedia](https://en.wikipedia.org/wiki/Wikipedia:Six_degrees_of_Wikipedia):

```
> path 2391008 4180390    // find a single path from
  2391008 : Coffee "Coffee"
   608699 : Age_of_Enlightenment "Age of Enlightenment"
  5758142 : K�nigsberg "K\u00F6nigsberg"
  4180390 : Graph_theory "Graph theory"
[0.053s]
> path-undirected 2391008 4180390  // find a single path, using both incoming and outgoing nodes
  2391008 : Coffee "Coffee"
  7891259 : Paul_Erdős "Paul Erd\u0151s"
  4180390 : Graph_theory "Graph theory"
[0.052s]
```

Both the `path` and `path-undirected` command can be run interactively to retrieve
multiple results, by using the * suffix:
```
> path-undirected* 2391008 4180390
  2391008 : Coffee "Coffee"
  7891259 : Paul_Erdős "Paul Erd\u0151s"
  4180390 : Graph_theory "Graph theory"
[n]ext/[a]bort: n
  2391008 : Coffee "Coffee"
    65825 : 18th_century "18th century"
  5952492 : Leonhard_Euler "Leonhard Euler"
  4180390 : Graph_theory "Graph theory"
[n]ext/[a]bort:      // or just hit enter
  2391008 : Coffee "Coffee"
   307264 : 2014_in_science "2014 in science"
  7275519 : Nature_(journal) "Nature (journal)"
  4180390 : Graph_theory "Graph theory"
[n]ext/[a]bort: a
```

Also, inputs/outputs can be inspected:
```
> outs 0
[ ->]   8116616 : Plus_and_minus_signs "Plus and minus signs"
[0s]
> ins 0
[0s]
```

## Data input

dbpedia reads "nt" files from the DBPedia project, which can be found here:
http://wiki.dbpedia.org/Downloads2015-04 . For early experiments, i would
recommend the "simple" english datasets, which only contain ~2.5 million
links.

dbpedia-server uses the following datasets:

- Titels (also called "labels"): This is required. It enables the functions
  `find-resource`, `find-label` and `find-id`.
- Page Links:
  enables graph and path commands.

Page categories and category relations (skos) is TBD.


## Performance characteristics

At the time of writing (2015-08), dbpedia exports 11.5M articles
and 162M links. System is
Intel(R) Core(TM) i7-4700MQ CPU @ 2.40GHz, HDD is Seagate ST1000LM014-SSHD-8GB (7200 RPM).

- Page labels (always required): ~50 seconds load time (1 decompressor and 4 parser threads), < 1GB RAM usage. 

- Page labels + outgoing page links: ~11 minutes load time, 2.3 GB virtual memory.

- Page labels + outgoing/incoming page links: ~15 minutes load time, 2.9GB virtual memory.

Path queries aren't thoruoghly benchmarked (yet). For a non-existant path,
the query requires ~2s to report failure. Succeeding queries typically run
in less than 0.05s:

```
> path 0 1
[1.575s]
> path-undirected 0 1
--- start path
        1 :    "++"
  4850983 : Increment_and_decrement_operators "Increment and decrement operators"
  8116617 : Plus_and_minus_signs "Plus and minus signs"
        0 :   "+"
--- end path
[0.029s]
```

Memory requirement per path is approximately `O(2*V*4 bytes + V*8 bytes)`, with V = the number of articles.
Removal of the `+ V * 8 bytes` part is possible (use a fixed-size (V) ringbuffer instead of a queue
for the work queue).

There is further optimzation and extension potential here, for example with using more sophisticated
graphing libraries such as Boost Graph, or the Threaded Boost Graph library.

## Other

- Coded using vim &amp; [YouCompleteMe](https://github.com/Valloric/YouCompleteMe)
- Entertainment by [The Glitch Mob](https://en.wikipedia.org/wiki/The_Glitch_Mob), [Deadmau5](https://en.wikipedia.org/wiki/Deadmau5),
  [Diablo Swing Orchestra](https://en.wikipedia.org/wiki/Diablo_Swing_Orchestra).

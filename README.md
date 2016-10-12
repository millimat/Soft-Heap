# Soft-Heap
Implementation of Kaplan and Zwick's soft heap, produced as a collaboration with Alex Hollender for CS 166 in Spring 2016.

This soft heap implementation is a translation into C of the pseudocode in Haim Kaplan and Uri Zwick's paper [A simpler implementation and analysis of Chazelle's Soft Heaps](http://dx.doi.org/10.1137/1.9781611973068.53). Kaplan and Zwick's data structure is an update to Bernard Chazelle's original soft heap, which he invented in 2000 in order to derive a minimum spanning tree algorithm which, as of 2016, has the fastest asymptotic running time of any MST algorithm with known complexity. (There is another, provably optimal algorithm with _unknown_ time complexity due to Pettie and Ramachandran.)

Our code has some bug corrections not accounted for in Kaplan and Zwick's original pseudocode, and we have included some tests to demonstrate the soft heap's performance as an approximate and exact sorter.

We wrote a paper in conjunction with this project: "Soft heaps: an intuitive overview." It's stored in this repo -- check it out for a full description of what soft heaps are and how they work.

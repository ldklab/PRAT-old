#!/usr/bin/env python3

import matplotlib.pyplot as plt
import networkx as nx

G = nx.Graph()
H = nx.Graph()

G.add_edges_from([(1,2), (1,3)])
G.add_node(1)
G.add_edge(1, 2)

H.add_edges_from([(3,4), (3,6)])
H.add_node(3)
#H.add_node(5)
H.add_edge(3,4)

print(nx.is_isomorphic(G, H))

# Draw test graph.
nx.draw(G)
nx.draw(H)
plt.show()
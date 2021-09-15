# file like
# u1 v1
# u2 v2
# ....
# transfrom into several files with same from
# every node should contain every adjacent edge
# if not directed, double the edge

from tqdm import tqdm
import argparse
import os


def genLines(u, vs):
    printlis = []
    vs = sorted(list(vs))
    for v in vs:
        edge = str(u) + ' ' + str(v) + '\n'
        printlis.append(edge)
    return printlis


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("-n", "--num", help="num of slices", required=True)
    parser.add_argument("-t", "--target", help="target graph", required=True)
    args = parser.parse_args()
    n = int(args.num)
    target = args.target
    directed = True
    # default wiki-vote
    path = "../datasets/wiki-vote/dataset/wiki-Vote.txt"
    givennodes = 7115
    if target == 'wk':
        pass    
    elif target == 'yt':
        path = "../datasets/youtube/dataset/com-youtube.ungraph.txt"
        givennodes = 1134890
    elif target == 'pt':
        path = "../datasets/patent/dataset/cit-Patents.txt"
        givennodes = 3774768
    elif target == 'lj':
        path = "../datasets/soc-livejournal/dataset/soc-LiveJournal1.txt"
        givennodes = 4847571
    elif target == 'tw':
        path = "../datasets/twitter/dataset/twitter-2010.txt"
        givennodes = 41652230
    elif target == 'fr':
        path = "../datasets/friendster/dataset/com-friendster.ungraph.txt"
        givennodes = 65608366
    else:
        path = target
        givennodes = 100000

    if not os.path.exists(target):
        os.mkdir(target)

    nodes = set()
    splitsize = givennodes * 1.1 / n
    with open(path, "r") as f:
        print("slicing")
        while 1:
            lines = f.readlines(1 << 30)
            if not lines:
                break
            else:
                files = []
                for i in range(n):
                    files.append(open(target + '/' + str(i), "a"))
                lines = tqdm(lines)

                for line in lines:
                    uv = line.split()
                    if not uv[0].isnumeric() or len(uv) != 2:
                        continue
                    u = int(uv[0])
                    v = int(uv[1])
                    if u == v:
                        continue
                    nodes.add(u)
                    nodes.add(v)
                    whichfile = int(u // splitsize) % n
                    files[whichfile].write(str(u) + " " + str(v) + "\n")
                    if not directed:
                        whichfile = int(v // splitsize) % n
                        files[whichfile].write(str(v) + " " + str(u) + "\n")

                for i in range(n):
                    files[i].close()

    print("slice done")
    nodes = sorted(list(nodes))
    maxnode = nodes[-1]
    print("maxnode: " + str(maxnode))
    mapping = [0] * (maxnode+1)
    for i in range(len(nodes)):
        mapping[nodes[i]] = i

    for i in range(n):
        files[i].close()

    for i in range(n):
        print("formatting file %d" % i)
        edge_count = 0
        printlines = []
        with open(target + '/' + str(i), "r+") as f:
            graph = {}
            lines = tqdm(f.readlines())
            for line in lines:
                uv = line.split()
                u = mapping[int(uv[0])]
                v = mapping[int(uv[1])]
                if u not in graph.keys():
                    graph[u] = set()
                graph[u].add(v)
            keys = sorted(list(graph.keys()))
            for key in keys:
                edge_count += len(graph[key])
                printlines += genLines(key, graph[key])
        with open(target + '/' + str(i), "w") as f:
            f.write(str(len(nodes)) + ' ' + str(edge_count) + '\n')
            f.writelines(printlines)

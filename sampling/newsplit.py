from tqdm import tqdm
import argparse


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
    if target == 'fr':
        path = "../datasets/friendster/dataset/com-friendster.ungraph.txt"
        givennodes = 65608366
    elif target == 'tw':
        path = "../datasets/twitter/dataset/twitter-2010.txt"
        givennodes = 41652230
    elif target == 'lj':
        path = "../datasets/soc-livejournal/dataset/soc-LiveJournal1.txt"
        givennodes = 4847571
    elif target == 'pt':
        path = "../datasets/patent/dataset/cit-Patents.txt"
        givennodes = 3774768
    elif target == 'yt':
        path = "../datasets/youtube/dataset/com-youtube.ungraph.txt"
        givennodes = 1134890

    splitsize = givennodes * 1.1 / n
    edges = [0] * n
    maxnode = 41652229
    with open(path, "r") as f:
        print("slicing")
        while 1:
            lines = f.readlines(1 << 30)
            if not lines:
                break
            else:
                files = []
                for i in range(n):
                    files.append(open(target + '/tmp' + str(i), "a"))
                lines = tqdm(lines)

                for line in lines:
                    uv = line.split()
                    if not uv[0].isnumeric() or len(uv) != 2:
                        continue
                    u = int(uv[0])
                    v = int(uv[1])
                    if u > maxnode:
                        maxnode = u
                    if v > maxnode:
                        maxnode = v
                    if u == v:
                        continue
                    whichfile = int(u // splitsize) % n
                    files[whichfile].write(str(u) + " " + str(v) + "\n")
                    edges[whichfile] += 1
                    if not directed:
                        whichfile = int(v // splitsize) % n
                        files[whichfile].write(str(v) + " " + str(u) + "\n")
                        edges[whichfile] += 1

                for i in range(n):
                    files[i].close()

    print("slice done")
    print("maxnode: " + str(maxnode))
    for i in range(n):
        print("edges in file " + str(i) + ": " + str(edges[i]))

    for i in range(1, n):
        print("formatting file %d" % i)
        edge_count = 0
        outfile = open(target + '/tmp' + str(i), "w")
        outfile.write(str(maxnode+1) + ' ' + str(edges[i]) + '\n')
        with open(target + '/' + str(i), "r") as f:
            graph = {}
            while 1:
                lines = f.readlines(1 << 30)
                if not lines:
                    break
                else:
                    lines = tqdm(lines)
                    for line in lines:
                        uv = line.split()
                        u = int(uv[0])
                        v = int(uv[1])
                        if u not in graph.keys():
                            graph[u] = set()
                        graph[u].add(v)
            keys = sorted(list(graph.keys()))
            for key in keys:
                edge_count += len(graph[key])
                outfile.writelines(genLines(key, graph[key]))
        print("edges: " + str(edge_count))
        outfile.close()

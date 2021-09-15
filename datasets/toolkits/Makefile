all: edgelist_to_biedgelist get_weighted_datasets

edgelist_to_biedgelist: edgelist_to_biedgelist.cpp
	g++ edgelist_to_biedgelist.cpp -o edgelist_to_biedgelist

get_weighted_datasets: get_weighted_datasets.cpp
	g++ get_weighted_datasets.cpp -o get_weighted_datasets

clean:
	rm edgelist_to_biedgelist get_weighted_datasets

# MPI 

If we run `mpirun` with `-mca orte_keep_fqdn_hostnames`, we can simply use node names with number of processes that will be ran on each node in each line.

For example, if we want to run our programm on 3 nodes, hostfile would look like this:

```bash
des01.kask slots=1
des02.kask slots=1
des03.kask slots=1
```
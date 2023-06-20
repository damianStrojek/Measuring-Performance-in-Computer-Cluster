# Delay between each iteration

This document outlines the potential to enhance the existing application by adding command line parameters for iteration control. 
Specifically, it proposes the addition of parameters that will allow users to customize the number of iterations and the delay between each iteration. 
This feature provides greater flexibility and adaptability to the application, enabling users to tailor the execution according to their specific needs.

The application currently lacks the ability to parameterize the number of iterations and the delay between each iteration. 
These values are typically hard-coded within the codebase, making it challenging for users to modify them without modifying the source code itself. 
This limitation restricts the application's versatility and hampers its compatibility with different use cases.

We could for an example run it with extra optional 2 parameters, one would be for delay[in seconds] and the other one for number of iterations

```bash
mpirun -mca orte_keep_fqdn_hostnames t -mca btl_tcp_if_exclude docker0,docker_gwbridge,lo -hostfile hostfile.des measure-performance --iterations 10 --delay 60
```

By adding command line parameters for iteration control, users gain the ability to customize the application's behavior according to their specific requirements.
This enhancement improves the application's flexibility, compatibility, and overall user experience. 
Implementing these parameters will allow users to easily parametrize the amount of iterations and the delay between each iteration, 
unlocking a wider range of potential use cases for the application.

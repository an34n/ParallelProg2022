### НИЯУ МИФИ. Лаботаторная работа #5. Андрюнькин Захар, Б20-505

# Используемая система

```
CPU: 6-Core Intel Core i7-9750H (-MT MCP-) speed: 2592 MHz 
Kernel: 5.10.16.3-microsoft-standard-WSL2 x86_64 
Up: 4h 47m Mem: 377.1/7859.7 MiB (4.8%) Storage: 512.33 GiB (149.5% used) 
Procs: 10 Shell: bash 5.0.17 inxi: 3.0.38

MPI:
HYDRA build details:
    Version:                                 4.0
    Release Date:                            Fri Jan 21 10:42:29 CST 2022
```

**Сложность алгоритма - O(n/p)**

n - количество элементов массива

p - количество потоков

```mermaid
    graph TD
        %%{ init : {"flowchart" : { "curve" : "stepAfter", "diagramPadding": 20 }}}%%
        A(Начало) --> B(i: 0 -> n)
        
        B --> C{"array[i] > max"}
        B --> E(Выход)
        C -->|Да| D["max = array[i]"]
        C -->|Нет| B
        D --> B
       
```

# Код
```
#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>

#define MROWS 100

int main(int argc, char** argv)
{
	int ret  = -1;	///< For return values
	int size = -1;	///< Total number of processors
	int rank = -1;	///< This processor's number

	const int count = 1e1; ///< Number of array elements
	// printf("DEFAULT VALUE - %d\n\n", DEBUG);
	const int random_seed = 1488; ///< RNG seed

	int* array = 0; ///< The array we need to find the max in
	int lmax = -1;	///< Local maximums
	int  max = -1;  ///< The maximal element

	double end, start;

	// * GENERATE TRACEFILE
    char filename[50];
    sprintf(filename, "trace.txt");
    FILE *fp = fopen(filename, "a");
    if (fp == NULL) {
        printf("Can't open file\n");
        exit(1);
    }

	/* Initialize the MPI */
	ret = MPI_Init(&argc, &argv);
	if (!rank) { printf("MPI Init returned (%d);\n", ret); }

	/* Determine our rankand processor count */
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	// printf("MPI Comm Size: %d;\n", size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	// printf("MPI Comm Rank: %d;\n", rank);

	/* Allocate the array */
	array = (int*)malloc(count * sizeof(int));

	// ! Master generates the array */
	if (!rank) {
		/* Initialize the RNG */
		srand(RANDOM_SEED);
		/* Generate the random array */
		for (int i = 0; i < count; i++) { array[i] = rand(); }
	}

	start = MPI_Wtime();

	//printf("Processor #%d has array: ", rank);
	//for (int i = 0; i < count; i++) { printf("%d ", array[i]); }
	//printf("\n");

	/* Send the array to all other processors */
	MPI_Bcast(array, count, MPI_INTEGER, 0, MPI_COMM_WORLD);

	// printf("Processor #%d has array: ", rank);
	// for (int i = 0; i < count; i++) { printf("%d ", array[i]); }
	// printf("\n");

	const int wstart = (rank    ) * count / size;
	const int wend   = (rank + 1) * count / size;

	// printf("Processor #%d checks items %d .. %d;\n", rank, wstart, wend - 1);

	for (int i = wstart;
		i < wend;
		i++)
	{
		if (array[i] > lmax) { lmax = array[i]; }
	}

	// printf("Processor #%d reports local max = %d;\n", rank, lmax);

	MPI_Reduce(&lmax, &max, 1, MPI_INTEGER, MPI_MAX, 0, MPI_COMM_WORLD);

	end = MPI_Wtime();
	ret = MPI_Finalize();
	// if (!rank) { 
	// 	printf("*** Global Maximum is %d;\n", max);
	// }

	fprintf(fp, "%lf;", end-start);
	// printf("MPI Finalize returned (%d);\n", ret);

	return(0);
}
```


```python
# REFACTOR DEFAULT VALUES FOR YOUR SYSTEM
CORE_LIMIT = 12
OLD_CUT = 10
NEW_CUT = 10
```


<details>
  <summary>Code</summary>
    ```python
    # import matplotlib.pyplot as plt
    import seaborn as sns
    import pandas as pd
    sns.set_theme(style="darkgrid")

    old_arr, cur_arr = [], []

    with open("old_trace.txt", "r") as fd:
        for _ in range(CORE_LIMIT):
            line = list(map(float, fd.readline()[:-2].split(";")))
            line = sorted(line)[OLD_CUT:-OLD_CUT]
            avg_value = (sum(line) / len(line))
            old_arr.append(avg_value)
    with open("trace.txt", "r") as fd:
        for i in range(CORE_LIMIT):
            line = list(map(float, fd.readline()[:-2].split(";")[::(i+1)]))
            line = sorted(line)[NEW_CUT:-NEW_CUT]
            avg_value = (sum(line) / len(line))
            cur_arr.append(avg_value)
    ```


    ```python
    for old, cur in zip(old_arr, cur_arr):
        print("{} - {}, delta - {}".format(round(old, 4), round(cur, 4), round(old-cur, 4)))
    ```

        0.0163 - 0.0119, delta - 0.0043
        0.0086 - 0.0672, delta - -0.0586
        0.0058 - 0.0957, delta - -0.0899
        0.0047 - 0.1089, delta - -0.1042
        0.0038 - 0.1202, delta - -0.1163
        0.0032 - 0.1297, delta - -0.1265
        0.0026 - 0.1374, delta - -0.1348
        0.0028 - 0.1398, delta - -0.137
        0.0026 - 0.1501, delta - -0.1475
        0.0024 - 0.1601, delta - -0.1577
        0.0022 - 0.1709, delta - -0.1687
        0.0021 - 0.178, delta - -0.176



    ```python
    # Create DataFrames
    old_predf = [[index+1, avg_time, "OpenMP"] for index, avg_time in enumerate(old_arr)]
    cur_predf = [[index+1, avg_time, "MPI"] for index, avg_time in enumerate(cur_arr)]
    old_df = pd.DataFrame(old_predf, columns=["Threads", "Time", "Type"])
    cur_df = pd.DataFrame(cur_predf, columns=["Threads", "Time", "Type"])

    all_df = pd.concat([old_df, cur_df])
    # all_df
    ```


    ```python
    # Time(Thread) graph
    p = sns.lineplot(x="Threads", y="Time", hue="Type", marker="o", data=all_df)
    p.set_xlabel("Threads, num", fontsize = 16)
    p.set_ylabel("Time, sec", fontsize = 16)
    l1 = p.lines[0]

    x1 = l1.get_xydata()[:, 0]
    y1 = l1.get_xydata()[:, 1]
    _ = p.fill_between(x1, y1, color="blue", alpha=0.3)
    p.margins(x=0, y=0)
    _ = p.set_xticks(range(0, CORE_LIMIT+1))
    _ = p.set_xticklabels([str(i) for  i in range(CORE_LIMIT+1)])

    ```


    

    



    ```python
    acceleration = [0] * CORE_LIMIT
    for i in range(0, CORE_LIMIT):
        acceleration[i] = [i+1, (cur_arr[0]/cur_arr[i])]
        

    a_df = pd.DataFrame(acceleration, columns=["Threads", "TimesSpeed"])
    p = sns.lineplot(x="Threads", y="TimesSpeed", marker="o", data=a_df, color='g')
    p.set_xlabel("Threads, num", fontsize = 16)
    p.set_ylabel("TimesSpeed, times", fontsize = 16)
    l1 = p.lines[0]

    x1 = l1.get_xydata()[:, 0]
    y1 = l1.get_xydata()[:, 1]

    _ = p.fill_between(x1, y1, color="green", alpha=0.3)
    _ = p.axvline(x=8, ymin=0.04, ymax=0.11, color="red", alpha=0.4)
    ```


    

    



    ```python
    per_thread = [0] * CORE_LIMIT
    for i in range(0, len(per_thread)):
        per_thread[i] = [acceleration[i][0], acceleration[i][1]/acceleration[i][0]]
    thr_df = pd.DataFrame(per_thread, columns=["Threads", "EfficencyPerThread"])
    p = sns.lineplot(x="Threads", y="EfficencyPerThread", marker="o", data=thr_df, color='b')
    p.set_xlabel("Threads, num", fontsize = 16)
    p.set_ylabel("EfficencyPerThread, times", fontsize = 16)
    l1 = p.lines[0]

    x1 = l1.get_xydata()[:, 0]
    y1 = l1.get_xydata()[:, 1]

    _ = p.fill_between(x1, y1, color="cyan", alpha=0.1)
    _ = p.axvline(x=6, ymin=0.04, ymax=0.05, color="red", alpha=0.4)
    ```
</details>

### Время исполнения (в сравнении с лабораторной #1)

![png](imgs/time.png)

### Эффективность на поток

![png](imgs/efficency.png)
    
### Ускорение на поток

![png](imgs/acceleration.png)
    


## Заключение

В этой работе я ознакомился с работой технологии **MPI**. В ходе работы была настроена рабочая среда, были оценены ускорение, эффективность и время работы параллельного алгоритма в зависимости от числа процессов. Также было произведено сравнение данной программы с работой программы на **OpenMP** из лаборатрной работы #1.

**Вывод:** Технология MPI в отличие от OpenMP не подразумевает использования общей памяти. Из-за чего тратит большее время на пересылку сообщений между параллельными процессами. Из-за того что программа довольно примитивна - время требуемое для пересылки сообщений больше чем время работы самой программы, поэтому в данном примере сравнительная эффективности технологии сильно ниже чем у OpenMP

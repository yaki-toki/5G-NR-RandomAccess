# RandomAccess
## 5G NR random access procedure simulator

~~Now working code is *"RandomAccessSimulator.c"* file.~~

### Finally simulator: *RandomAccessSimulatorBeta.c*

### You should create a folder in the same path as below before executing the code.
~~~bash
mkdir ./Uniform_SimulationResults
mkdir ./Beta_SimulationResults
~~~

#### Simulation parameters:
- Default parameter:
    - nUE: [5000, 10000, 20000, ..., 100000]
    - Distribution: Uniform, Beta
    - Simulation time: Uniform (60s), Beta (10s)
        - A unit of time: ms
- Variable parameter:
    - Number of preambles: 54, 64, ...
    - Backof indicator: 20 ms (Random backoff)

## Simulation results

### Number of preamble: 54
| Number of devices per cell   | 10,000 | 20,000 | 30,000 | 40,000 | 50,000 | 60,000 | 70,000 | 80,000 | 90,000 | 100,000 |
|------------------------------|--------|--------|--------|--------|--------|--------|--------|--------|--------|---------|
| Success ratio                |1.00    |0.99    |0.72    |0.50    |0.39    |0.32    |0.27    |0.23    |0.21    |0.18     |
| Number of successful devices |10000   |19721   |21745   |19820   |19290   |19026   |18769   |18602   |18674   |18415    |
| Number of preamble tx        |1.20    |1.60    |1.76    |1.55    |1.46    |1.40    |1.34    |1.31    |1.30    |1.26     |
| Access delay (ms)            |10.59ms |16.91ms |19.53ms |16.16ms |14.81ms |13.76ms |12.77ms |12.24ms |12.14ms |11.55ms  |

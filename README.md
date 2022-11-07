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
    - Number of UL grant for one RAR: 12, ...

## Simulation results
### Simulation results by Number of preambles
#### Number of preamble: 54
| Number of devices per cell   | 10,000 | 20,000 | 30,000 | 40,000 | 50,000 | 60,000 | 70,000 | 80,000 | 90,000 | 100,000 |
|------------------------------|--------|--------|--------|--------|--------|--------|--------|--------|--------|---------|
| Success ratio                |1.00    |0.99    |0.72    |0.50    |0.39    |0.32    |0.27    |0.23    |0.21    |0.18     |
| Number of successful devices |10,000  |19,721  |21,745  |19,820  |19,290  |19,026  |18,769  |18,602  |18,674  |18,415   |
| Number of preamble tx        |1.20    |1.60    |1.76    |1.55    |1.46    |1.40    |1.34    |1.31    |1.30    |1.26     |
| Access delay (ms)            |10.59   |16.91   |19.53   |16.16   |14.81   |13.76   |12.77   |12.24   |12.14   |11.55    |

#### Number of preamble: 64

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
| Success ratio                |100   |89.292|60.928|46.2|37.252|31.283|26.889|23.621|21.079 |18.989 |
| Number of successful devices |10,000  |19,721  |21,745  |19,820  |19,290  |19,026  |18,769  |18,602  |18,674  |18,415   |
| Number of preamble tx        |2.59    |5.358   |5.523   |5.603   |5.651   |5.686   |5.709   |5.73    |5.748   |5.76     |
| Access delay (ms)            |47.114  |89.65   |92.254  |93.515  |94.289  |94.849  |95.205  |95.548  |95.819  |96.001   |
									
#### Number of preamble: 64
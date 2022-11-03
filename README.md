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
- nUE: [5000, 10000, 20000, ..., 100000]
- Distribution: Uniform, Beta
- Simulation time: Uniform (60s), Beta (10s)
    - A unit of time: ms
- Number of preambles: 64
- Backof indicator: 20 ms (Random backoff)
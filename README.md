# RandomAccess
## 5G NR random access procedure simulator

### Finally simulator: *RandomAccessSimulatorBeta.c*

### You should create a folder in the same path as below before executing the code.
~~~bash
mkdir ./BasicUniformSimulationResults
mkdir ./BasicBetaSimulationResults
~~~

### Simulation Environment Define

![Simulation Environment](./assets/SimulationEnvironmentDefine.png)

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

## Run code

### Code compile

~~~bash
gcc RandomAccessWithNOMA.c -o Simulator
~~~

### Run code

~~~bash
./Simulator
~~~

### Run code parameters

~~~
--times         -t : Simulation times (int)
                     Simulation count must be greater than zero.
                     Default 1

--distribution  -d : Traffic model (1 or 2)
                     1: traffic model 1 (Uniform distribution)
                     2: traffic model 2 (Beta distribution)

--preambles     -p : Number of preambles (int)
                     Number of preamble must be greater than zero.
                     Default 54

--backoff       -b : Backoff indicator (int)
                     Backoff indicator must be greater than zero.
                     Default 20

--grant         -g : The number of Up Link Grant per RAR (int)
                     The number of Up Link Grant per RAR must be greater than zero.
                     Default 12

--rarCount      -r : RAR window size (int)
                     The maximum RAR window size must be greater than zero.
                     Default 5

--maxRar        -m : Maximum retransmission (int)
                     Maximum retransmissions must be greater than zero.
                     Default 10

--subframe      -s : Subframe units (int)
                     The size of the subframe must be at least 5. (float)
                     Default 5

--cell          -c : Cell radius Size
                     The radius of the cell is entered in diameter units and must be greater than 400m.
                     Default 400.0

--hbs           -b : Height of BS from ground (float)
                     The height of the BS must be between 10m and 20m.
                     Default 10.0

--hut           -u : Height of UE from ground (float)
                     The height of the UE must be between 1.5m and 22.5m.
                     Default 1.8
~~~

## Simulation results
### Simulation results by Number of retransmission limit
#### Retransmission limit: 10
| Number of devices per cell   | 10,000 | 20,000 | 30,000 | 40,000 | 50,000 | 60,000 | 70,000 | 80,000 | 90,000 | 100,000 |
|------------------------------|--------|--------|--------|--------|--------|--------|--------|--------|--------|---------|
| Success ratio                |100   |89.292|60.928|46.2|37.252|31.283|26.889|23.621|21.079 |18.989 |
| Number of successful devices |10,000  |19,721  |21,745  |19,820  |19,290  |19,026  |18,769  |18,602  |18,674  |18,415   |
| Number of preamble tx        |2.59    |5.358   |5.523   |5.603   |5.651   |5.686   |5.709   |5.73    |5.748   |5.76     |
| Access delay (ms)            |47.114  |89.65   |92.254  |93.515  |94.289  |94.849  |95.205  |95.548  |95.819  |96.001   |

#### Retransmission limit: 20
| Number of devices per cell   | 10,000 | 20,000 | 30,000 | 40,000 | 50,000 | 60,000 | 70,000 | 80,000 | 90,000 | 100,000 |
|------------------------------|--------|--------|--------|--------|--------|--------|--------|--------|--------|---------|
| Success ratio                |100     |  89.548|	60.912|	 46.279|  37.284|  31.306|  26.944|  23.612|  21.079|	18.993|
| Number of successful devices |10,000  |  17,909|	18,274|  18,513|  18,642|  18,782|  18,861|  18,890|  18,973|	18,994| 
| Number of preamble tx        |2.561   |     9.6|  10.062|  10.269|  10.371|  10.454| 	10.531|  10.576|  10.617|    10.65|
| Access delay (ms)            |45.366|	155.95|162.953|166.348|168.099|169.489|170.487|171.054|172.027|172.233|

#### Retransmission limit: 50
| Number of devices per cell   | 10,000 | 20,000 | 30,000 | 40,000 | 50,000 | 60,000 | 70,000 | 80,000 | 90,000 | 100,000 |
|------------------------------|--------|--------|--------|--------|--------|--------|--------|--------|--------|---------|
| Success ratio                |100|	89.451|	60.95|	46.251|	37.298|	31.287|	26.946|	23.673|	21.021|	19.012|
| Number of successful devices |10,000| 	17,890| 	18,285| 	18,501| 	18,649| 	18,772| 	18,862| 	18,939| 	18,920| 	19,011| 
| Number of preamble tx        |3.541|	22.279|	23.34|	24.052|	24.342|	24.671|	24.631|	24.718|	25.028|	25.22|
| Access delay (ms)            |58.844|	347.504|	363.314|	374.048|	379.32|	384.342|	383.852|	385.134|	389.256|	392.61|

### Simulation results based on the number of retransmissions

![Simulation results based on the number of retransmissions](./assets/SimulationResultsBasedOnTheNumberOfRetransmissions.png)
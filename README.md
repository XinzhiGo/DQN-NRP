## How to run

### dependencies

- libtorch
- spdlog

Download libtorch from the official website, and read the installation guide.

- [Installing C++ Distributions of PyTorch — PyTorch main documentation](https://pytorch.org/cppdocs/installing.html)

### Compile

The project uses cmake to compile the source code. Execute the following commands to compile the project.

```bash
cmake -S . -B build
cmake --build build
```

Executable files will be generated in the `bin` directory, library files will be generated in the `lib` directory.

Run the executable file to train the model.

## How to train

Uncomment the following line in `Solution.cpp` to train the model. Run the executable file to train the model.

```cpp
// save the network weights
q_net_.save_weights(q_net_path);
q_target_.save_weights(q_target_path);
```

## How to run a demo

If you want to run a specific instance, like the "n100w4_1_4_4_2_8" in paper. You can change the instanceName, initHis, and weekdata in simulator/main.cpp debugRun function. 

- instanceName = "n100w4"
- initHis = 1
- weekdata = {4,4,2,8}

## How to run comparison algorithm
To validate the effectiveness of DQN, you can change the biasTabuSearch Function in `solver.cpp`.

- sln.dqnSearch -------------- DQN-ILS
- sln.compareSearch ------------ ANTS
- sln.VNDSearch -------------- VNDTS

Furthermore, if you want to change the neighborhood structure, you can change the ModeSeq.

## References

- INRC2 Competition website: http://mobiz.vives.be/inrc2/

## Bibliography

```bibtex
@inproceedings{
  title={Deep Q-Network-Based Neighborhood Tabu Search for Nurse Rostering Problem},
  author={Zhang, Xinzhi and Zhu, Qingling and Lin, Qiuzhen and Chen, Wei-Neng and Li, Jianqiang and Coello Coello, Carlos Artemio},
  journal={Applied soft computing}
}
```

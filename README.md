# The ESBMC model checker

[![Codacy Badge](https://api.codacy.com/project/badge/Grade/d14d06e975644907a2eb9521e09ccfe4)](https://app.codacy.com/gh/esbmc/esbmc?utm_source=github.com&utm_medium=referral&utm_content=esbmc/esbmc&utm_campaign=Badge_Grade_Dashboard)
![Lint Code Base](https://github.com/esbmc/esbmc/workflows/Lint%20Code%20Base/badge.svg)
![Health Checks](https://github.com/esbmc/esbmc/workflows/Health%20Checks/badge.svg)
![Build All Solvers](https://github.com/esbmc/esbmc/workflows/Build%20All%20Solvers/badge.svg)
[![codecov](https://codecov.io/gh/esbmc/esbmc/branch/master/graph/badge.svg)](https://codecov.io/gh/esbmc/esbmc)

ESBMC (the Efficient SMT-based Context-Bounded Model Checker) is a mature, permissively licensed open-source context-bounded model checker for verifying single- and multithreaded C/C++, CUDA, CHERI, Kotlin, Python, and Solidity programs. It can automatically verify predefined safety properties (e.g., bounds check, pointer safety, overflow) and user-defined program assertions. 

ESBMC supports: 

- The Clang compiler as its C/C++/CHERI/CUDA frontend;
- The Soot framework via Jimple as its Java/Kotlin frontend;
- The ast2json package as its [Python frontend](./src/python-frontend/README.md);
- Implements the Solidity grammar production rules as its Solidity frontend;
- Supports IEEE floating-point arithmetic for various SMT solvers.

ESBMC also implements state-of-the-art incremental BMC and *k*-induction proof-rule algorithms based on Satisfiability Modulo Theories (SMT) and Constraint Programming (CP) solvers.

We recommend you read some background material/publications to understand exactly what ESBMC can provide, which are available [online](https://ssvlab.github.io/esbmc/publications.html).

Our main website is [esbmc.org](http://esbmc.org). 

### Architecture

The figure below illustrates the current ESBMC architecture. The tool inputs a C/C++/CUDA, Java/Kotlin, Solidity, or CHERI-C program, then converts an abstract syntax tree (AST) into a state transition system called a GOTO program. Its symbolic execution engine unrolls the GOTO program and generates a sequence of static single assignments (SSAs). The SSAs are then converted to an SMT formula, which is satisfiable if and only if the program contains errors.

![esbmc-architecture-v3](https://github.com/esbmc/esbmc/assets/3694109/fe609179-14bc-4f3d-b507-a3b003f732ae)

### How to build/install ESBMC

To compile ESBMC on Ubuntu 24.04 with LLVM 14 and the SMT solver Z3:

```
sudo apt install clang-14 llvm-14 clang-tidy-14 python-is-python3 python3 git ccache unzip wget curl bison flex g++-multilib linux-libc-dev libboost-all-dev libz3-dev libclang-14-dev libclang-cpp-dev cmake
git clone https://github.com/esbmc/esbmc.git
mkdir build && cd build
cmake ../esbmc -DENABLE_Z3=1
make -j4
```

To build ESBMC with other operating systems and SMT solvers, please see the [BUILDING](https://github.com/esbmc/esbmc/blob/master/BUILDING.md) file. 

The user can also download the latest ESBMC binary for Ubuntu and Windows from the [releases page](https://github.com/esbmc/esbmc/releases).

### How to use ESBMC

As an illustrative example to show some of the ESBMC features, consider the following C code:

````C
#include <math.h>
int main() {
  unsigned int N = nondet_uint();
  double x = nondet_double();
  if(x <= 0 || isnan(x))
    return 0;
  unsigned int i = 0;
  while(i < N) {
    x = (2*x);
    assert(x>0);
    ++i;
  }
  assert(x>0);
  return 0;
}
````

Here, ESBMC is invoked as follows:

````
$esbmc file.c --k-induction
````

Where `file.c` is the C program to be checked, and --k-induction selects the k-induction proof rule. The user can choose the SMT solver, property, and verification strategy. For this particular C program, ESBMC provides the following output as the verification result:

````
*** Checking inductive step
Starting Bounded Model Checking
Unwinding loop 2 iteration 1 file ex5.c line 8 function main
Not unwinding loop 2 iteration 2 file ex5.c line 8 function main
Symex completed in: 0.001s (40 assignments)
Slicing time: 0.000s (removed 16 assignments)
Generated 2 VCC(s), 2 remaining after simplification (24 assignments)
No solver specified; defaulting to Boolector
Encoding remaining VCC(s) using bit-vector/floating-point arithmetic
Encoding to solver time: 0.005s
Solving with solver Boolector 3.2.0
Encoding to solver time: 0.005s
Runtime decision procedure: 0.427s
BMC program time: 0.435s

VERIFICATION SUCCESSFUL

Solution found by the inductive step (k = 2)
````

We refer the user to our [documentation webpage](https://ssvlab.github.io/esbmc/documentation.html) for further examples of the ESBMC's features.

### Features

ESBMC detects errors in software by simulating a finite prefix of the program execution with all possible inputs. Classes of implementation errors that can be detected include:
 * User-specified assertion failures
 * Out-of-bounds array access
 * Illegal pointer dereferences, such as:
   * Dereferencing null
   * Performing an out-of-bounds dereference
   * Double-free of malloc'd memory
   * Misaligned memory access
 * Integer overflows
 * Undefined behavior on shift operations
 * Floating-point for NaN
 * Divide by zero
 * Memory leaks

Concurrent software (using the pthread api) is verified by explicitly exploring interleavings, producing one symbolic execution per interleaving. By default, pointer-safety, array-out-of-bounds, division-by-zero, and user-specified assertions will be checked for; one can also specify options to check concurrent programs for:
 * Deadlock (only on pthread mutexes and convars)
 * Data races (i.e. competing writes)
 * Atomicity violations at visible assignments
 * Lock acquisition ordering

By default, ESBMC performs a "lazy" depth-first search of interleavings -- it can also encode (explicitly) all interleavings into a single SMT formula.

Many SMT solvers are currently supported:
 * Z3 4.8+
 * Bitwuzla
 * Boolector 3.0+
 * MathSAT
 * CVC4
 * CVC5
 * Yices 2.2+

In addition, ESBMC can be configured to use the SMTLIB interactive text format with a pipe to communicate with an arbitrary solver process, although not-insignificant overheads are involved.

A limited subset of C++98/03 is supported, too -- a library modeling the STL is also available.

### Tutorials

We provide a short video that explains ESBMC: 

https://www.youtube.com/watch?v=uJ5Jn0sxm08&t=2182s

This video was delivered as part of a technical talk on exploiting the SAT revolution for automated software verification in a workshop between Arm Research and the University of Manchester.

We offer a post-graduate course in software security that explains the internals of ESBMC. 

https://ssvlab.github.io/lucasccordeiro/courses/2020/01/software-security/index.html

This course unit introduces students to basic and advanced approaches to formally building verified trustworthy software systems, where trustworthiness comprises five attributes: *reliability*, *availability*, *safety*, *resilience* and *security*.

### Selected Publications

* Yiannis Charalambous, Norbert Tihanyi, Ridhi Jain, Youcheng Sun, Mohamed Amine Ferrag, Lucas C. Cordeiro. [A New Era in Software Security: Towards Self-Healing Software via Large Language Models and Formal Verification](https://arxiv.org/pdf/2305.14752.pdf). Technical Report, CoRR abs/2305.14752, 2023. [DOI](https://doi.org/10.48550/arXiv.2305.14752)
  
* Rafael Menezes, Daniel Moura, Helena Cavalcante, Rosiane de Freitas, Lucas C. Cordeiro . [ESBMC-Jimple: verifying Kotlin programs via jimple intermediate representation](https://dl.acm.org/doi/abs/10.1145/3533767.3543294) In ISSTA'22, pp. 777-780, 2022. [DOI](https://doi.org/10.1145/3533767.3543294)

* Franz Brauße, Fedor Shmarov, Rafael Menezes, Mikhail R. Gadelha, Konstantin Korovin, Giles Reger, Lucas C. Cordeiro. [ESBMC-CHERI: towards verification of C programs for CHERI platforms with ESBMC](https://dl.acm.org/doi/abs/10.1145/3533767.3543289) In ISSTA'22, pp. 773-776, 2022. [DOI](https://doi.org/10.1145/3533767.3543289) 

* Felipe R. Monteiro, Mikhail R. Gadelha, Lucas C. Cordeiro. [Model checking C++ programs.](https://onlinelibrary.wiley.com/doi/epdf/10.1002/stvr.1793) In Softw. Test. Verification Reliab. 32(1), 2022. [DOI](https://doi.org/10.1002/stvr.1793), [Video](https://www.youtube.com/watch?v=cX31c976tjM), **Open access**.

* Mikhail R. Gadelha, Lucas C. Cordeiro, Denis A. Nicole. [An Efficient Floating-Point Bit-Blasting API for Verifying C Programs.](https://ssvlab.github.io/lucasccordeiro/papers/nsv2020.pdf) In VSTTE, pp. 178-195, 2020. [DOI](https://doi.org/10.1007/978-3-030-63618-0_11)

* Mikhail Y. R. Gadelha, Felipe R. Monteiro, Jeremy Morse, Lucas C. Cordeiro, Bernd Fischer, Denis A. Nicole. [ESBMC 5.0: an industrial-strength C model checker.](https://ssvlab.github.io/lucasccordeiro/papers/ase2018.pdf) In ASE, pp. 888-891, 2018. [DOI](https://doi.org/10.1145/3238147.3240481) 

* Jeremy Morse, Lucas C. Cordeiro, Denis A. Nicole, Bernd Fischer. [Model checking LTL properties over ANSI-C programs with bounded traces.](https://ssvlab.github.io/lucasccordeiro/papers/sosym2013.pdf) In Softw. Syst. Model. 14(1), pp. 65-81, 2015. [DOI](https://doi.org/10.1007/s10270-013-0366-0)

* Mikhail Y. R. Gadelha, Hussama Ibrahim Ismail, Lucas C. Cordeiro. [Handling loops in bounded model checking of C programs via k-induction.](https://ssvlab.github.io/lucasccordeiro/papers/sttt2017.pdf) In Int. J. Softw. Tools Technol. Transf. 19(1), pp. 97-114, 2017. [DOI](https://doi.org/10.1007/s10009-015-0407-9)

* Phillipe A. Pereira, Higo F. Albuquerque, Isabela da Silva, Hendrio Marques, Felipe R. Monteiro, Ricardo Ferreira, Lucas C. Cordeiro. [SMT-based context-bounded model checking for CUDA programs.](https://ssvlab.github.io/lucasccordeiro/papers/cppe2017.pdf) In Concurr. Comput. Pract. Exp. 29(22), 2017. [DOI](https://doi.org/10.1002/cpe.3934)

* Lucas C. Cordeiro, Bernd Fischer, João Marques-Silva. [SMT-Based Bounded Model Checking for Embedded ANSI-C Software.](https://ssvlab.github.io/lucasccordeiro/papers/tse2012.pdf) In IEEE Trans. Software Eng. 38(4), pp. 957-974, 2012. [DOI](https://doi.org/10.1109/TSE.2011.59)

* Lucas C. Cordeiro, Bernd Fischer. [Verifying multi-threaded software using smt-based context-bounded model checking.](https://ssvlab.github.io/lucasccordeiro/papers/icse2011.pdf) In ICSE, pp. 331-340, 2011. [DOI](https://doi.org/10.1145/1985793.1985839)

### Awards

* Distinguished Paper Award at ICSE’11
* Best Paper Award at SBESC’15
* Most Influential Paper Award at ASE’23
* Best Tool Paper Award at SBSeg'23
* 35 awards from international competitions on software verification (SV-COMP) and testing (Test-Comp) 2012-2024 at TACAS/FASE (Strength: Bug Finding and Code Coverage).

### ESBMC-CHERI Video & Download

This [video](https://youtu.be/CsWHnmU4UMs) describes how to obtain, build and run ESBMC-CHERI on an example.

A pre-compiled binary for Linux is available in the pre-release
[ESBMC-CHERI](https://github.com/esbmc/esbmc/releases/tag/v6.9-cheri), for other
systems/archs the [BUILDING.md](https://github.com/esbmc/esbmc/blob/cheri-clang/BUILDING.md)
document explains the necessary installation steps.

### Differences from CBMC

ESBMC is a fork of CBMC v2.9 (2008), the C Bounded Model Checker. The primary differences between the two are:

* CBMC focuses on SAT-based encodings of unrolled programs, while ESBMC targets SMT-based encodings.
* CBMC's concurrency support is an entirely symbolic encoding of a concurrent program in one SAT formula, while ESBMC explores each interleaving individually using context-bounded verification.
* CBMC uses a modified C parser written by James Roskind and a C++ parser based on OpenC++, while ESBMC relies on the Clang front-end.
* ESBMC implements the Solidity grammar production rules as its Solidity frontend, while CBMC does not implement a Solidity frontend.
* ESBMC verifies Kotlin programs with a model of the standard Kotlin libraries and checks a set of safety properties, while CBMC cannot handle Kotlin programs.
* CBMC implements k-induction, requiring three different calls: to generate the CFG, to annotate the program, and to verify it, whereas ESBMC handles the whole process in a single call. Additionally, CBMC does not have a forward condition to check if all states were reached and relies on a limited loop unwinding.
* ESBMC adds some additional types to the program's internal representation.

# Open source

ESBMC has now been released as open-source software, mainly distributed under the terms of the Apache License 2.0. ESBMC contains a significant amount of other people's software. However, please see the COPYING file for an explanation of who owns what and under what terms it is distributed.

We'd be extremely happy to receive contributions to improve ESBMC (under the terms of the Apache License 2.0). If you'd like to submit anything, please file a pull request against the public GitHub repo. General discussion and release announcements will be made via GitHub. Please post an issue on GitHub and contact us about research or collaboration.

### Getting started

We need a better guide for getting started with ESBMC, although we hope to improve this in the future. Examining some of the benchmarks in the SV-COMP competition (http://sv-comp.sosy-lab.org/) would be a good start, using the ESBMC command line for the relevant competition year.

You can also explore practical examples using the ESBMC tool on the documentation page, which is available at https://ssvlab.github.io/esbmc/documentation.html.

### Contributing to the code base

Here are some steps to contributing to the code base:

  1. Compile and execute esbmc. [Building](https://github.com/esbmc/esbmc/blob/master/BUILDING.md)
  1. Fork the repository
  1. Clone the repository git clone git@github.com:YOURNAME/esbmc.git
  1. Create a branch from the master branch (default branch)
  1. Make your changes
  1. Check the formatting with clang-format (use Clang 9)
  1. Push your changes to your branch
  1. Create a Pull Request targeting the master branch

Here is an example of preparing a pull request (PR)


A) Ensure you are in the `master` branch and your fork is updated.

```
git checkout master
git fetch upstream
git pull --rebase upstream master
git push origin HEAD:master
```

Note that if you have not yet set up the `upstream`, you need to type the following command:

```
git remote add upstream https://github.com/esbmc/esbmc
```

B) Create a local branch (e.g., `model-pthread-create`) from the `master` branch:

```
git checkout -b model-pthread-equal --track master
```

C) Add your changes via commits to the local branch:

```
git add path-to-file/file.cpp
git commit -sm "added opertational model for pthread_equal"
```

Note that you can check your changes via `git status`.
Note also that every PR should contain at least two test cases
to check your implementation: one successful and one failed test case.

D) Push your changes in the local branch to the ESBMC repository:

```
git push origin model-pthread-equal
```

New contributors can check issues marked with `good first issue` by clicking [here](https://github.com/esbmc/esbmc/contribute).

### Documentation

A limited number of classes have been marked up with doxygen documentation headers. Comments are put in the header files declaring classes and methods. HTML documentation can be generated by running:

    doxygen .doxygen

The output will be in docs/HTML; open index.html to get started.

### Acknowledgments

ESBMC is a joint project with the Federal University of Amazonas (Brazil), University of Manchester (UK), University of Southampton (UK), and University of Stellenbosch (South Africa).

The ESBMC development was supported by various research funding agencies, including CNPq (Brazil), CAPES (Brazil), FAPEAM (Brazil), EPSRC (UK), Royal Society (UK), British Council (UK), European Commission (Horizon 2020), and companies including ARM, Intel, Motorola Mobility, Nokia Institute of Technology and Samsung. The ESBMC development is currently funded by ARM, EPSRC grants [EP/T026995/1](https://enncore.github.io) and [EP/V000497/1](https://scorch-project.github.io), [Ethereum Foundation](https://blog.ethereum.org/2022/07/29/academic-grants-grantee-announce), [EU H2020 ELEGANT 957286](https://www.elegant-h2020.eu), Intel, Motorola Mobility (through Agreement N° 4/2021), and [Soteria project](https://soteriaresearch.org) awarded by the UK Research and Innovation for the Digital Security by Design (DSbD) Programme.

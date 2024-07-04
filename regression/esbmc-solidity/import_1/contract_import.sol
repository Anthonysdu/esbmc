// SPDX-License-Identifier: GPL-3.0
pragma solidity >=0.8.0;

contract A {
    function func_1() public virtual returns (int8) {
        return 21;
    }
}

contract B is A {
    function func_1() public override pure returns (int8) {
        return 42;
    }
}
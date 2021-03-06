/*
 * Copyright (c) 2017, 2018, Oracle and/or its affiliates. All rights reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3 only, as
 * published by the Free Software Foundation.
 *
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 3 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 *
 * You should have received a copy of the GNU General Public License version
 * 3 along with this work; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Please contact Oracle, 500 Oracle Parkway, Redwood Shores, CA 94065 USA
 * or visit www.oracle.com if you need additional information or have any
 * questions.
 */
package com.oracle.truffle.r.runtime.interop;

import com.oracle.truffle.api.dsl.Fallback;
import com.oracle.truffle.api.dsl.Specialization;
import com.oracle.truffle.r.runtime.RRuntime;
import com.oracle.truffle.r.runtime.data.RInteropScalar.RInteropByte;
import com.oracle.truffle.r.runtime.data.RInteropScalar.RInteropChar;
import com.oracle.truffle.r.runtime.data.RInteropScalar.RInteropFloat;
import com.oracle.truffle.r.runtime.data.RInteropScalar.RInteropLong;
import com.oracle.truffle.r.runtime.data.RInteropScalar.RInteropShort;
import com.oracle.truffle.r.runtime.data.RRaw;
import com.oracle.truffle.r.runtime.data.model.RAbstractDoubleVector;
import com.oracle.truffle.r.runtime.data.model.RAbstractIntVector;
import com.oracle.truffle.r.runtime.data.model.RAbstractLogicalVector;
import com.oracle.truffle.r.runtime.data.model.RAbstractRawVector;
import com.oracle.truffle.r.runtime.data.model.RAbstractStringVector;
import com.oracle.truffle.r.runtime.nodes.RBaseNode;

public abstract class R2Foreign extends RBaseNode {

    public abstract Object execute(Object obj);

    @Specialization
    public boolean doByte(byte obj) {
        return RRuntime.fromLogical(obj);
    }

    @Specialization()
    public double doDouble(double vec) {
        return vec;
    }

    @Specialization()
    public int doInt(int vec) {
        return vec;
    }

    @Specialization()
    public String doString(String vec) {
        return vec;
    }

    @Specialization()
    public byte doRaw(RRaw vec) {
        return vec.getValue();
    }

    @Specialization(guards = "vec.getLength() == 1")
    public double doDoubleVector(RAbstractDoubleVector vec) {
        return vec.getDataAt(0);
    }

    @Specialization(guards = "vec.getLength() == 1")
    public int doIntVector(RAbstractIntVector vec) {
        return vec.getDataAt(0);
    }

    @Specialization(guards = "vec.getLength() == 1")
    public boolean doLogicalVector(RAbstractLogicalVector vec) {
        return vec.getDataAt(0) == RRuntime.LOGICAL_TRUE;
    }

    @Specialization(guards = "vec.getLength() == 1")
    public byte doRawVector(RAbstractRawVector vec) {
        return vec.getRawDataAt(0);
    }

    @Specialization(guards = "vec.getLength() == 1")
    public String doStringVector(RAbstractStringVector vec) {
        return vec.getDataAt(0);
    }

    @Specialization
    public byte doInteroptByte(RInteropByte obj) {
        return obj.getValue();
    }

    @Specialization
    public char doInteroptChar(RInteropChar obj) {
        return obj.getValue();
    }

    @Specialization
    public float doInteroptFloat(RInteropFloat obj) {
        return obj.getValue();
    }

    @Specialization
    public long doInteroptLong(RInteropLong obj) {
        return obj.getValue();
    }

    @Specialization
    public short doInteroptShort(RInteropShort obj) {
        return obj.getValue();
    }

    @Fallback
    public static Object doObject(Object obj) {
        return obj;
    }

    public static R2Foreign create() {
        return R2ForeignNodeGen.create();
    }
}

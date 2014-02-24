/*
 * Copyright (c) 2013, 2014, Oracle and/or its affiliates. All rights reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.
 *
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 *
 * You should have received a copy of the GNU General Public License version
 * 2 along with this work; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Please contact Oracle, 500 Oracle Parkway, Redwood Shores, CA 94065 USA
 * or visit www.oracle.com if you need additional information or have any
 * questions.
 */
package com.oracle.truffle.r.runtime.data;

import java.util.*;

import com.oracle.truffle.api.CompilerDirectives.SlowPath;
import com.oracle.truffle.r.runtime.*;
import com.oracle.truffle.r.runtime.data.model.*;

public final class RRawVector extends RVector implements RAbstractRawVector {

    private byte[] data;

    RRawVector(byte[] data, int[] dims, Object names) {
        super(true, data.length, dims, names);
        this.data = data;
    }

    RRawVector(byte[] data, int[] dims) {
        this(data, dims, null);
    }

    @Override
    protected RRawVector internalCopy() {
        return new RRawVector(Arrays.copyOf(data, data.length), null);
    }

    @Override
    protected int internalGetLength() {
        return data.length;
    }

    @Override
    @SlowPath
    public String toString() {
        return Arrays.toString(data);
    }

    @Override
    protected boolean internalVerify() {
        return true;
    }

    public RRaw getDataAt(int i) {
        return RDataFactory.createRaw(data[i]);
    }

    public byte[] getDataCopy() {
        byte[] copy = new byte[data.length];
        System.arraycopy(data, 0, copy, 0, data.length);
        return copy;
    }

    public RRawVector copyWithNewDimensions(int[] newDimensions) {
        return RDataFactory.createRawVector(data, newDimensions);
    }

    public RRawVector removeLast() {
        assert getLength() > 0;
        return RDataFactory.createRawVector(Arrays.copyOf(data, getLength() - 1));
    }

    public RRawVector removeFirst() {
        assert getLength() > 0;
        return RDataFactory.createRawVector(Arrays.copyOfRange(data, 1, getLength()));
    }

    @Override
    protected String getDataAtAsString(int index) {
        return getDataAt(index).toString();
    }

    public RRawVector materialize() {
        return this;
    }

    public RRawVector updateDataAt(int i, RRaw right) {
        assert !this.isShared();
        data[i] = right.getValue();
        return this;
    }

    private byte[] createResizedData(int size, boolean fillNA) {
        assert !this.isShared();
        byte[] newData = Arrays.copyOf(data, size);
        if (!fillNA) {
            // NA is 00 for raw
            for (int i = data.length, j = 0; i < size; ++i, j = Utils.incMod(j, data.length)) {
                newData[i] = data[j];
            }
        }
        return newData;
    }

    @Override
    public RRawVector copyResized(int size, boolean fillNA) {
        return RDataFactory.createRawVector(createResizedData(size, fillNA));
    }

    @Override
    protected void resizeInternal(int size) {
        this.data = createResizedData(size, true);
    }

    @Override
    public RRawVector createEmptySameType(int newLength, boolean newIsComplete) {
        assert newIsComplete == true;
        return RDataFactory.createRawVector(new byte[newLength]);
    }

    @Override
    public void transferElementSameType(int toIndex, RVector fromVector, int fromIndex) {
        RRawVector other = (RRawVector) fromVector;
        data[toIndex] = other.data[fromIndex];
    }

    public Class<?> getElementClass() {
        return RRaw.class;
    }

    @Override
    public Object getDataAtAsObject(int index) {
        return getDataAt(index);
    }

    @Override
    public List<String> getClassHierarchy() {
        final List<String> classHr = super.getClassHierarchy();
        classHr.add(RRuntime.TYPE_RAW);
        return classHr;
    }
}

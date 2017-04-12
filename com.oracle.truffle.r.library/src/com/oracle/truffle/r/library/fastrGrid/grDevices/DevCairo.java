/*
 * Copyright (c) 2017, Oracle and/or its affiliates. All rights reserved.
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
package com.oracle.truffle.r.library.fastrGrid.grDevices;

import com.oracle.truffle.r.library.fastrGrid.GridContext;
import com.oracle.truffle.r.library.fastrGrid.device.SVGDevice;
import com.oracle.truffle.r.nodes.builtin.RExternalBuiltinNode;
import com.oracle.truffle.r.runtime.RError.Message;
import com.oracle.truffle.r.runtime.RRuntime;
import com.oracle.truffle.r.runtime.data.RArgsValuesAndNames;
import com.oracle.truffle.r.runtime.data.RNull;

public class DevCairo extends RExternalBuiltinNode {
    static {
        Casts.noCasts(DevCairo.class);
    }

    @Override
    protected Object call(RArgsValuesAndNames args) {
        if (args.getLength() < 4) {
            throw error(Message.ARGUMENTS_REQUIRED_COUNT, args.getLength(), "devCairo", 4);
        }

        String filename = RRuntime.asString(args.getArgument(0));
        int witdh = RRuntime.asInteger(args.getArgument(2));
        int height = RRuntime.asInteger(args.getArgument(2));
        if (RRuntime.isNA(witdh) || RRuntime.isNA(height) || RRuntime.isNA(filename) || filename.isEmpty()) {
            throw error(Message.INVALID_ARG_TYPE);
        }

        GridContext.getContext().setCurrentDevice("svg", new SVGDevice(filename, (double) witdh / 72., (double) height / 72.));
        return RNull.instance;
    }
}

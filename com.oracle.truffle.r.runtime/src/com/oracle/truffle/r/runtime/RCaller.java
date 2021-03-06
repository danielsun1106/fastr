/*
 * Copyright (c) 2014, 2018, Oracle and/or its affiliates. All rights reserved.
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
package com.oracle.truffle.r.runtime;

import java.util.function.Supplier;

import com.oracle.truffle.api.frame.Frame;
import com.oracle.truffle.r.runtime.nodes.RSyntaxElement;

/**
 * Represents the caller of a function and stored in {@link RArguments}. A value of this type never
 * appears in a Truffle execution. Caller remembers its parent caller and frame number as described
 * in {@code sys.parent} R function documentation: frames are numbered from 0 (global environment),
 * parent does not have to have the frame with number one less, e.g. with do.call(fun, args, envir)
 * when fun asks for parent, it should get 'envir', moreover, when evaluating promises parent frame
 * and frame with number one less are typically also not the same frames. See also builtins in
 * {@code FrameFunctions} for more details.
 *
 * NOTE: It is important to create new caller instances for each stack frame, so that
 * {@link ReturnException#getTarget()} can uniquely identify the target frame.
 * 
 * @see RArguments
 */
public final class RCaller {

    public static final RCaller topLevel = RCaller.createInvalid(null);

    /**
     * Determines the actual position of the corresponding frame on the execution call stack. When
     * one follows the {@link RCaller#parent} chain, then the depth is not always decreasing by only
     * one, the reason are promises, which may be evaluated somewhere deep down the call stack, but
     * their parent call frame from R prespective could be much higher up the actual execution call
     * stack.
     */
    private final int depth;
    private boolean visibility;
    private final RCaller parent;
    /**
     * The payload can be an RSyntaxNode, a {@link Supplier}, or an {@link RCaller} (which marks
     * promise evaluation frames). Payload represents the syntax (AST) of how the function was
     * invoked. If the function was invoked via regular call node, then the syntax can be that call
     * node (RSyntaxNode case), if the function was invoked by other means and we do not have the
     * actual syntax for the invocation, we only provide it lazily via Supplier, so that we do not
     * have to always construct the AST nodes.
     */
    private final Object payload;

    private RCaller(Frame callingFrame, Object nodeOrSupplier) {
        this.depth = depthFromFrame(callingFrame);
        this.parent = parentFromFrame(callingFrame);
        this.payload = nodeOrSupplier;
    }

    private RCaller(int depth, RCaller parent, Object nodeOrSupplier) {
        this.depth = depth;
        this.parent = parent;
        this.payload = nodeOrSupplier;
    }

    private static int depthFromFrame(Frame callingFrame) {
        return callingFrame == null ? 0 : RArguments.getCall(callingFrame).getDepth() + 1;
    }

    private static RCaller parentFromFrame(Frame callingFrame) {
        return callingFrame == null ? null : RArguments.getCall(callingFrame);
    }

    public int getDepth() {
        return depth;
    }

    public RCaller getParent() {
        return parent;
    }

    public RSyntaxElement getSyntaxNode() {
        assert payload != null && !(payload instanceof RCaller) : payload == null ? "null RCaller" : "promise RCaller";
        return payload instanceof RSyntaxElement ? (RSyntaxElement) payload : (RSyntaxElement) ((Supplier<?>) payload).get();
    }

    public boolean isValidCaller() {
        return payload != null;
    }

    public boolean isPromise() {
        return payload instanceof RCaller;
    }

    public RCaller getPromiseOriginalCall() {
        return (RCaller) payload;
    }

    public static RCaller createInvalid(Frame callingFrame) {
        return new RCaller(callingFrame, null);
    }

    public static RCaller createInvalid(Frame callingFrame, RCaller parent) {
        return new RCaller(depthFromFrame(callingFrame), parent, null);
    }

    public static RCaller create(Frame callingFrame, RSyntaxElement node) {
        assert node != null;
        return new RCaller(callingFrame, node);
    }

    public static RCaller create(Frame callingFrame, RCaller parent, RSyntaxElement node) {
        assert node != null;
        return new RCaller(depthFromFrame(callingFrame), parent, node);
    }

    public static RCaller create(Frame callingFrame, Supplier<RSyntaxElement> supplier) {
        assert supplier != null;
        return new RCaller(callingFrame, supplier);
    }

    public static RCaller create(int depth, RCaller parent, Object payload) {
        assert payload != null;
        return new RCaller(depth, parent, payload);
    }

    public static RCaller create(Frame callingFrame, RCaller parent, Supplier<RSyntaxElement> supplier) {
        assert supplier != null;
        return new RCaller(depthFromFrame(callingFrame), parent, supplier);
    }

    public static RCaller createForPromise(RCaller originalCaller, Frame frame) {
        int newDepth = frame == null ? 0 : RArguments.getDepth(frame);
        RCaller originalCall = frame == null ? null : RArguments.getCall(frame);
        return new RCaller(newDepth, originalCaller, originalCall);
    }

    public boolean getVisibility() {
        return visibility;
    }

    public void setVisibility(boolean visibility) {
        this.visibility = visibility;
    }
}

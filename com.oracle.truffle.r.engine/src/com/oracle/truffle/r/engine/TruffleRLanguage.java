/*
 * Copyright (c) 2015, 2016, Oracle and/or its affiliates. All rights reserved.
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
package com.oracle.truffle.r.engine;

import java.io.IOException;
import java.util.Locale;

import com.oracle.truffle.api.CallTarget;
import com.oracle.truffle.api.TruffleLanguage;
import com.oracle.truffle.api.frame.MaterializedFrame;
import com.oracle.truffle.api.instrument.Visualizer;
import com.oracle.truffle.api.instrument.WrapperNode;
import com.oracle.truffle.api.nodes.Node;
import com.oracle.truffle.api.source.Source;
import com.oracle.truffle.api.vm.PolyglotEngine;
import com.oracle.truffle.r.nodes.RASTBuilder;
import com.oracle.truffle.r.nodes.builtin.RBuiltinPackages;
import com.oracle.truffle.r.nodes.instrumentation.NewInstrumentFactory;
import com.oracle.truffle.r.runtime.FastROptions;
import com.oracle.truffle.r.runtime.RAccuracyInfo;
import com.oracle.truffle.r.runtime.RError;
import com.oracle.truffle.r.runtime.RInternalError;
import com.oracle.truffle.r.runtime.RPerfStats;
import com.oracle.truffle.r.runtime.RRuntime;
import com.oracle.truffle.r.runtime.RVersionInfo;
import com.oracle.truffle.r.runtime.TempPathName;
import com.oracle.truffle.r.runtime.context.Engine.IncompleteSourceException;
import com.oracle.truffle.r.runtime.context.Engine.ParseException;
import com.oracle.truffle.r.runtime.context.RContext;
import com.oracle.truffle.r.runtime.ffi.Load_RFFIFactory;
import com.oracle.truffle.r.runtime.ffi.RFFIFactory;

/**
 * Only does the minimum for running under the debugger. It is not completely clear how to correctly
 * integrate the R startup in {@code RCommand} with this API.
 */
@TruffleLanguage.Registration(name = "R", version = "0.1", mimeType = {RRuntime.R_APP_MIME, RRuntime.R_TEXT_MIME})
public final class TruffleRLanguage extends TruffleLanguage<RContext> {

    /**
     * The choice of {@link RFFIFactory} is made statically so that it is bound into an AOT-compiled
     * VM. The decision is node made directly in {@link RFFIFactory} to avoid some project
     * dependencies that cause build problems.
     */
    private static void initialize() {
        try {
            Load_RFFIFactory.initialize(true);
            RPerfStats.initialize();
            Locale.setDefault(Locale.ROOT);
            RAccuracyInfo.initialize();
            RVersionInfo.initialize();
            TempPathName.initialize();
            RContext.initialize(new RASTBuilder(), new RRuntimeASTAccessImpl(), RBuiltinPackages.getInstance());
        } catch (Throwable t) {
            System.out.println("error during engine initialization:");
            t.printStackTrace();
            System.exit(-1);
        }
    }

    private static boolean initialized;

    public static final TruffleRLanguage INSTANCE = new TruffleRLanguage();

    public static final String MIME = RRuntime.R_APP_MIME;

    private TruffleRLanguage() {
    }

    @Override
    protected boolean isObjectOfLanguage(Object object) {
        return false;
    }

    @Override
    protected RContext createContext(Env env) {
        // Currently using env.instrumenter as "initialized" flag
        boolean initialContext = !initialized;
        if (!initialized) {
            FastROptions.initialize();
            initialize();
            initialized = true;
        }
        RContext result = RContext.create(env, new NewInstrumentFactory(env), initialContext);
        return result;
    }

    @Override
    protected void disposeContext(RContext context) {
        context.destroy();
    }

    @Override
    protected String toString(RContext context, Object value) {
        // TODO This is a hack because R is still printing its own results
        // every use of PolyglotEngine should return a value instead of printing the result.
        return null;
    }

    @Override
    protected CallTarget parse(Source source, Node context, String... argumentNames) throws IOException {
        try {
            return RContext.getEngine().parseToCallTarget(source);
        } catch (IncompleteSourceException e) {
            throw new com.oracle.truffle.api.vm.IncompleteSourceException(e);
        } catch (ParseException e) {
            return new CallTarget() {
                @Override
                public Object call(Object... arguments) {
                    try {
                        throw e.throwAsRError();
                    } catch (@SuppressWarnings("hiding") RError e) {
                        return null;
                    }
                }
            };
        }
    }

    @Override
    protected Object findExportedSymbol(RContext context, String globalName, boolean onlyExplicit) {
        return context.getExportedSymbols().get(globalName);
    }

    @Override
    protected Object getLanguageGlobal(RContext context) {
        // TODO: what's the meaning of "language global" for R?
        return null;
    }

    // TODO: why isn't the original method public?
    public Node actuallyCreateFindContextNode() {
        return createFindContextNode();
    }

    @SuppressWarnings("deprecation")
    @Override
    protected Visualizer getVisualizer() {
        return null;
    }

    @Override
    protected boolean isInstrumentable(Node node) {
        throw RInternalError.shouldNotReachHere();
    }

    @Override
    protected WrapperNode createWrapperNode(Node node) {
        throw RInternalError.shouldNotReachHere();
    }

    @Override
    protected Object evalInContext(Source source, Node node, MaterializedFrame frame) throws IOException {
        return RContext.getEngine().parseAndEval(source, frame, false);
    }

    /**
     * Temporary workaround until {@link PolyglotEngine} provides a way to invoke
     * {@link #evalInContext}.
     */
    public Object internalEvalInContext(Source source, Node node, MaterializedFrame frame) throws IOException {
        return evalInContext(source, node, frame);
    }

}

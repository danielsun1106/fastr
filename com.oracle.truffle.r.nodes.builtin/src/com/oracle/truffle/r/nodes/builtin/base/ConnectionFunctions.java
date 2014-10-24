/*
 * Copyright (c) 2014, 2014, Oracle and/or its affiliates. All rights reserved.
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
package com.oracle.truffle.r.nodes.builtin.base;

import static com.oracle.truffle.r.runtime.RBuiltinKind.*;

import java.io.*;
import java.util.*;
import java.util.zip.*;

import com.oracle.truffle.api.CompilerDirectives.TruffleBoundary;
import com.oracle.truffle.api.dsl.*;
import com.oracle.truffle.api.source.*;
import com.oracle.truffle.r.nodes.builtin.*;
import com.oracle.truffle.r.runtime.*;
import com.oracle.truffle.r.runtime.RContext.ConsoleHandler;
import com.oracle.truffle.r.runtime.data.*;
import com.oracle.truffle.r.runtime.data.model.*;
import com.oracle.truffle.r.runtime.env.*;

public abstract class ConnectionFunctions {
    private static class ModeException extends IOException {
        private static final long serialVersionUID = 1L;

        private ModeException() {
            // lame but it's what GnuR does
            super("invalid argument");
        }
    }

    private enum OpenMode {
        Lazy(new String[]{""}),
        Read(new String[]{"r", "rt"}),
        Write(new String[]{"w", "wt"}),
        Append(new String[]{"a", "at"}),
        ReadBinary(new String[]{"rb"}),
        WriteBinary(new String[]{"wb"}),
        AppendBinary(new String[]{"ab"}),
        ReadWrite(new String[]{"r+"}),
        ReadWriteBinary(new String[]{"r+b"}),
        ReadWriteTrunc(new String[]{"w+"}),
        ReadWriteTruncBinary(new String[]{"w+b"}),
        ReadAppend(new String[]{"a+"}),
        ReadAppendBinary(new String[]{"a+b"});

        private String[] modeStrings;

        OpenMode(String[] modeStrings) {
            this.modeStrings = modeStrings;
        }

        @TruffleBoundary
        static OpenMode getOpenMode(String modeString) throws IOException {
            for (OpenMode openMode : values()) {
                for (String ms : openMode.modeStrings) {
                    if (ms.equals(modeString)) {
                        return openMode;
                    }
                }
            }
            throw new ModeException();
        }
    }

    // TODO remove invisibility when print for RConnection works
    // TODO implement all open modes
    // TODO implement missing .Internal functions expected by connections.R

    /**
     * Base class for all {@link RConnection} instances. It supports lazy opening, as required by
     * the R spec, through the {@link #theConnection} field, which ultimately holds the actual
     * connection instance when opened. The default implementation of the {@link RConnection}
     * methods check whether the connection is open and if not, calls
     * {@link #createDelegateConnection()} and then forwards the operation. A subclass may choose
     * not to use delegation by overriding the default implementations.
     *
     */
    private static abstract class BaseRConnection extends RConnection {
        protected boolean isOpen;
        /**
         * if {@link #isOpen} is {@code true} the {@link OpenMode} that this connection is opened
         * in, otherwise {@link OpenMode#Lazy}.
         */
        protected OpenMode mode;
        /**
         * If {@link #isOpen} is {@code false} and {@link OpenMode} is {@link OpenMode#Lazy}, the
         * mode the connection should eventually be opned in.
         */
        private OpenMode lazyMode;

        private RStringVector classHr;

        /**
         * The actual connection, if delegated.
         */
        private DelegateRConnection theConnection;

        protected BaseRConnection(String modeString) throws IOException {
            this(modeString, OpenMode.Read);
        }

        protected BaseRConnection(String modeString, OpenMode lazyMode) throws IOException {
            this(OpenMode.getOpenMode(modeString), lazyMode);
        }

        protected BaseRConnection(OpenMode mode, OpenMode lazyMode) {
            this.mode = mode;
            this.lazyMode = lazyMode;
        }

        protected void checkOpen() throws IOException {
            if (!isOpen) {
                // closed or lazy
                if (mode == OpenMode.Lazy) {
                    mode = lazyMode;
                    createDelegateConnection();
                }
            }
        }

        protected void setDelegate(DelegateRConnection conn, String rClass) {
            this.theConnection = conn;
            setClass(rClass);
            isOpen = true;
        }

        protected void setClass(String rClass) {
            String classes[] = new String[2];
            classes[0] = rClass;
            classes[1] = "connection";
            this.classHr = RDataFactory.createStringVector(classes, RDataFactory.COMPLETE_VECTOR);
        }

        @Override
        public String[] readLines(int n) throws IOException {
            checkOpen();
            return theConnection.readLines(n);
        }

        @Override
        public InputStream getInputStream() throws IOException {
            checkOpen();
            return theConnection.getInputStream();
        }

        @Override
        public void close() throws IOException {
            isOpen = false;
            theConnection.close();
        }

        @Override
        public RStringVector getClassHierarchy() {
            return classHr;
        }

        /**
         * Subclass-specific creation of the {@link DelegateRConnection} using {@link #mode}. To
         * support both lazy and non-lazy creation, the implementation of this method must
         * explicitly call {@link #setDelegate(DelegateRConnection, String)} if it successfully
         * creates the delegate connection.
         */
        protected abstract void createDelegateConnection() throws IOException;
    }

    private static abstract class DelegateRConnection extends RConnection {
        protected BaseRConnection base;

        DelegateRConnection(BaseRConnection base) {
            this.base = base;
        }

        @Override
        public RStringVector getClassHierarchy() {
            return base.getClassHierarchy();
        }

    }

    /**
     * A special case that does not use delegation as the connection is always open.
     */
    private static class StdinConnection extends BaseRConnection {

        StdinConnection() {
            super(OpenMode.Read, null);
            this.isOpen = true;
            setClass("terminal");
        }

        @Override
        @TruffleBoundary
        public String[] readLines(int n) throws IOException {
            ConsoleHandler console = RContext.getInstance().getConsoleHandler();
            ArrayList<String> lines = new ArrayList<>();
            String line;
            while ((line = console.readLine()) != null) {
                lines.add(line);
                if (n > 0 && lines.size() == n) {
                    break;
                }
            }
            String[] result = new String[lines.size()];
            lines.toArray(result);
            return result;
        }

        @Override
        public InputStream getInputStream() throws IOException {
            throw RInternalError.shouldNotReachHere();
        }

        @Override
        public void close() throws IOException {
            throw RInternalError.shouldNotReachHere();
        }

        @Override
        protected void createDelegateConnection() throws IOException {
            // nothing to do, as we override the RConnection methods directly.
        }

    }

    private static StdinConnection stdin;

    @RBuiltin(name = "stdin", kind = INTERNAL, parameterNames = {})
    public abstract static class Stdin extends RInvisibleBuiltinNode {
        @Specialization
        @TruffleBoundary
        protected RConnection stdin() {
            controlVisibility();
            if (stdin == null) {
                stdin = new StdinConnection();
            }
            return stdin;
        }
    }

    /**
     * Base class for all modes of file connections.
     *
     */
    private static class FileRConnection extends BaseRConnection {
        protected final String path;

        protected FileRConnection(String path, String modeString) throws IOException {
            super(modeString);
            this.path = path;
            if (mode != OpenMode.Lazy) {
                createDelegateConnection();
            }
        }

        @Override
        protected void createDelegateConnection() throws IOException {
            DelegateRConnection delegate = null;
            switch (mode) {
                case Read:
                    delegate = new FileReadTextRConnection(this);
                    break;
                default:
                    throw RError.nyi((SourceSection) null, "unimplemented open mode: " + mode);
            }
            setDelegate(delegate, "file");
        }
    }

    private static class FileReadTextRConnection extends DelegateRConnection {
        private BufferedReader bufferedReader;

        FileReadTextRConnection(FileRConnection base) throws IOException {
            super(base);
            bufferedReader = new BufferedReader(new FileReader(base.path));
        }

        @TruffleBoundary
        @Override
        public String[] readLines(int n) throws IOException {
            ArrayList<String> lines = new ArrayList<>();
            String line;
            while ((line = bufferedReader.readLine()) != null) {
                lines.add(line);
                if (n > 0 && lines.size() == n) {
                    break;
                }
            }
            String[] result = new String[lines.size()];
            lines.toArray(result);
            return result;
        }

        @Override
        public InputStream getInputStream() throws IOException {
            throw new IOException();
        }

        @Override
        public void close() throws IOException {
            bufferedReader.close();
        }
    }

    @RBuiltin(name = "file", kind = INTERNAL, parameterNames = {"description", "open", "blocking", "encoding", "raw"})
    public abstract static class File extends RInvisibleBuiltinNode {
        @Specialization
        @TruffleBoundary
        @SuppressWarnings("unused")
        protected Object file(RAbstractStringVector description, RAbstractStringVector open, byte blocking, RAbstractStringVector encoding, byte raw) {
            // temporarily return invisible to avoid missing print support
            controlVisibility();
            try {
                return new FileRConnection(description.getDataAt(0), open.getDataAt(0));
            } catch (IOException ex) {
                RError.warning(RError.Message.CANNOT_OPEN_FILE, description.getDataAt(0), ex.getMessage());
                throw RError.error(getEncapsulatingSourceSection(), RError.Message.CANNOT_OPEN_CONNECTION);
            }
        }

        @SuppressWarnings("unused")
        @Fallback
        protected Object file(Object description, Object open, Object blocking, Object encoding, Object raw) {
            controlVisibility();
            throw RError.error(getEncapsulatingSourceSection(), RError.Message.INVALID_UNNAMED_ARGUMENTS);
        }
    }

    /**
     * Base class for all modes of gzfile connections.
     */
    private static class GZIPRConnection extends BaseRConnection {
        protected final String path;

        GZIPRConnection(String path, String modeString) throws IOException {
            super(modeString, OpenMode.ReadBinary);
            this.path = Utils.tildeExpand(path);
            if (mode != OpenMode.Lazy) {
                createDelegateConnection();
            }
        }

        @Override
        protected void createDelegateConnection() throws IOException {
            DelegateRConnection delegate = null;
            switch (mode) {
                case ReadBinary:
                    delegate = new GZIPInputRConnection(this);
                    break;
                default:
                    throw RError.nyi((SourceSection) null, "unimplemented open mode: " + mode);
            }
            setDelegate(delegate, "gzfile");
        }
    }

    private static class GZIPInputRConnection extends DelegateRConnection {
        private GZIPInputStream stream;

        GZIPInputRConnection(GZIPRConnection base) throws IOException {
            super(base);
            stream = new GZIPInputStream(new FileInputStream(base.path));
        }

        @Override
        public String[] readLines(int n) throws IOException {
            throw new IOException("TODO");
        }

        @Override
        public InputStream getInputStream() throws IOException {
            return stream;
        }

        @Override
        public void close() throws IOException {
            stream.close();
        }

    }

    @RBuiltin(name = "gzfile", kind = INTERNAL, parameterNames = {"description", "open", "encoding", "compression"})
    public abstract static class GZFile extends RInvisibleBuiltinNode {
        @Specialization
        @TruffleBoundary
        @SuppressWarnings("unused")
        protected Object gzFile(RAbstractStringVector description, RAbstractStringVector open, RAbstractStringVector encoding, double compression) {
            // temporarily return invisible to avoid missing print support
            controlVisibility();
            try {
                return new GZIPRConnection(description.getDataAt(0), open.getDataAt(0));
            } catch (IOException ex) {
                RError.warning(RError.Message.CANNOT_OPEN_FILE, description.getDataAt(0), ex.getMessage());
                throw RError.error(getEncapsulatingSourceSection(), RError.Message.CANNOT_OPEN_CONNECTION);
            }
        }
    }

    /**
     * Base class for all modes of gzfile connections.
     */
    private static class TextRConnection extends BaseRConnection {
        @SuppressWarnings("unused") protected String nm;
        protected RAbstractStringVector object;
        @SuppressWarnings("unused") protected REnvironment env;

        protected TextRConnection(String nm, RAbstractStringVector object, REnvironment env, String modeString) throws IOException {
            super(modeString);
            this.nm = nm;
            this.object = object;
            this.env = env;
            if (mode != OpenMode.Lazy) {
                createDelegateConnection();
            }
        }

        @Override
        protected void createDelegateConnection() throws IOException {
            DelegateRConnection delegate = null;
            switch (mode) {
                case Read:
                    delegate = new TextReadRConnection(this);
                    break;
                default:
                    throw RError.nyi((SourceSection) null, "unimplemented open mode: " + mode);
            }
            setDelegate(delegate, "textConnection");
        }
    }

    private static class TextReadRConnection extends DelegateRConnection {
        private String[] lines;
        private int index;

        TextReadRConnection(TextRConnection base) {
            super(base);
            StringBuffer sb = new StringBuffer();
            for (int i = 0; i < base.object.getLength(); i++) {
                sb.append(base.object.getDataAt(i));
                // vector elements are implicitly terminated with a newline
                sb.append('\n');
            }
            lines = sb.toString().split("\\n");
        }

        @Override
        public String[] readLines(int n) throws IOException {
            int nleft = lines.length - index;
            int nlines = nleft;
            if (n > 0) {
                nlines = n > nleft ? nleft : n;
            }

            String[] result = new String[nlines];
            System.arraycopy(lines, index, result, 0, nlines);
            index += nlines;
            return result;
        }

        @Override
        public InputStream getInputStream() throws IOException {
            throw RInternalError.shouldNotReachHere();
        }

        @Override
        public void close() throws IOException {
        }
    }

    @RBuiltin(name = "textConnection", kind = INTERNAL, parameterNames = {"nm", "object", "open", "env", "type"})
    public abstract static class TextConnection extends RInvisibleBuiltinNode {
        @Specialization
        @TruffleBoundary
        protected Object textConnection(RAbstractStringVector nm, RAbstractStringVector object, RAbstractStringVector open, REnvironment env, @SuppressWarnings("unused") RIntVector encoding) {
            controlVisibility();
            try {
                return new TextRConnection(nm.getDataAt(0), object, env, open.getDataAt(0));
            } catch (IOException ex) {
                throw RInternalError.shouldNotReachHere();
            }
        }
    }

    @RBuiltin(name = "close", kind = INTERNAL, parameterNames = {"con", "..."})
    public abstract static class Close extends RInvisibleBuiltinNode {
        @Specialization
        @TruffleBoundary
        protected Object close(Object con) {
            controlVisibility();
            if (con instanceof RConnection) {
                try {
                    ((RConnection) con).close();
                } catch (IOException e) {
                    throw RInternalError.unimplemented();
                }
                return RNull.instance;
            } else {
                throw RError.error(getEncapsulatingSourceSection(), RError.Message.NOT_CONNECTION, "con");
            }
        }
    }

    @RBuiltin(name = "readLines", kind = INTERNAL, parameterNames = {"con", "n", "ok", "warn", "encoding", "skipNul"})
    public abstract static class ReadLines extends RBuiltinNode {
        @Specialization
        @TruffleBoundary
        protected Object readLines(RConnection con, int n, byte ok, @SuppressWarnings("unused") byte warn, @SuppressWarnings("unused") String encoding, @SuppressWarnings("unused") byte skipNul) {
            controlVisibility();
            try {
                String[] lines = con.readLines(n);
                if (n > 0 && lines.length < n && ok == RRuntime.LOGICAL_FALSE) {
                    throw RError.error(getEncapsulatingSourceSection(), RError.Message.TOO_FEW_LINES_READ_LINES);
                }
                return RDataFactory.createStringVector(lines, RDataFactory.COMPLETE_VECTOR);
            } catch (IOException x) {
                throw RError.error(getEncapsulatingSourceSection(), RError.Message.ERROR_READING_CONNECTION, x.getMessage());
            }
        }

        @Specialization
        @TruffleBoundary
        protected Object readLines(RConnection con, double n, byte ok, byte warn, String encoding, byte skipNul) {
            return readLines(con, (int) n, ok, warn, encoding, skipNul);
        }

        @SuppressWarnings("unused")
        @Fallback
        protected Object readLines(Object con, Object n, Object ok, Object warn, Object encoding, Object skipNul) {
            controlVisibility();
            throw RError.error(getEncapsulatingSourceSection(), RError.Message.INVALID_UNNAMED_ARGUMENTS);
        }
    }

}

#include "ngx_nrpc_handler.h"
#include "protocol.h"

namespace nrpc {

void ngx_nrpc_init_connection(ngx_connection_t *c)
{
    auto rpc_session = new RpcSession(c);
    if (!rpc_session->init()) {
        return rpc_session->finalize();
    }
    c->data = (void*)rpc_session;

    c->read->handler = ngx_nrpc_determine_policy;
    return ngx_nrpc_determine_policy(c->read);
}

// read one byte to determin policy
// just finalize when (timeout | eof | error)
void ngx_nrpc_determine_policy(ngx_event_t *rev)
{
    ngx_connection_t *c = (ngx_connection_t*)rev->data;
    RpcSession* rpc_session = (RpcSession*)c->data;

    if (rev->timedout) {
        c->timedout = 1;
        rpc_session->set_result(RPC_READ_TIMEOUT);
        return rpc_session->finalize();
    }

    char first_character;
    int n = c->recv(c, (u_char*)&first_character, 1);
    if(n == NGX_AGAIN) {
        if (!rev->timer_set) {
            ngx_add_timer(rev, rpc_session->read_timeout());
        }
        if (ngx_handle_read_event(rev, 0) != NGX_OK) {
            rpc_session->set_result(RPC_INNER_ERROR);
            rpc_session->set_result_text("hanlde read event error");
            return rpc_session->finalize();
        }
        // for next event
        return;
    }

    if((n == 0) || (n == NGX_ERROR)) {
        // read eof or error
        rpc_session->set_result(RPC_READ_ERROR);
        rpc_session->finalize();
    }

    // n == 1
    c->received += 1;
    int protocol_num;
    switch(first_character) {
    case 'N':
        protocol_num = NRPC_PROTOCOL_DEFAULT_NUM/*default*/;
        break;
    case 'G':
    case 'P':
    case 'H':
        protocol_num = NRPC_PROTOCOL_HTTP_NUM/*http*/;
        break;
    default:
        protocol_num = 100/*invaild*/;
        break;
    }
    if (!rpc_session->set_protocol(protocol_num/*default_protocol*/)) {
        return rpc_session->finalize();
    }

    ngxplus::IOBuf* iobuf = new ngxplus::IOBuf(); // default blocksize 1024
    char* buf;
    int len = iobuf->alloc(&buf, 1, ngxplus::IOBuf::IOBUF_ALLOC_EXACT);
    if (len != 1) {
        rpc_session->set_result(RPC_INNER_ERROR);
        rpc_session->set_result_text("determin alloc error");
        return rpc_session->finalize();
    }
    buf[0] = first_character;
    rpc_session->set_iobuf(iobuf);

    rev->handler = ngx_nrpc_read_request;
    return ngx_nrpc_read_request(rev);
}

void ngx_nrpc_read_request(ngx_event_t *rev)
{
    ngx_connection_t *c = (ngx_connection_t*)rev->data;
    RpcSession *session = (RpcSession*)c->data;

    if (rev->timedout) {
        c->timedout = 1;
        session->set_result(RPC_READ_TIMEOUT);
        return session->finalize();
    }

    RPC_SESSION_STATE session_state = session->state();
    if (session_state != RPC_SESSION_READING_REQUEST) {
        session->set_result(RPC_INNER_ERROR);
        session->set_result_text("should not come into read request");
        return session->finalize();
    }

    ngxplus::IOBuf *iobuf = session->iobuf();
    ngxplus::IOBufAsZeroCopyInputStream zero_in_stream(iobuf);

    char* buf;
    int size;
    for(;;) {
        if (!zero_in_stream.Next((const void**)&buf, &size)) {
            session->set_result(RPC_INNER_ERROR);
            session->set_result_text("read request alloc error");
            session->finalize();
        }
        if (size == 0) {
            continue;
        }
        int len = c->recv(c, (u_char*)buf, size);
        if (len == size) {
            continue;
        }
        if (len == NGX_ERROR) {
            session->set_result(RPC_READ_ERROR);
            return session->finalize();
        }

        // read eof or again
        bool read_eof = false;
        if (len == 0) {
            read_eof = true;
        }
        ParseResult presult = session->protocol()->parse(iobuf, session, read_eof);
        if (presult == PARSE_DONE) {
            rev->handler = ngx_nrpc_dummy_read;
            ngx_del_timer(rev);
            session->set_state(RPC_SESSION_PROCESSING);
            return session->protocol()->process_request(session,
                    *((DefaultProtocolCtx*)session->protocol_ctx())->rpc_meta, iobuf);
        }
        else if (presult == PARSE_INCOMPLETE) {
            ngx_add_timer(rev, session->read_timeout());
            if (ngx_handle_read_event(rev, 0) != NGX_OK) {
                session->set_result(RPC_INNER_ERROR);
                session->set_result_text("hanlde read event error");
                return session->finalize();
            }
            return;
        }
        else { // bad format OR broken OR bad meta
            session->set_result(RPC_READ_ERROR);
            if (presult == PARSE_BROKEN) {
                session->set_result_text("read broken");
            }
            return session->finalize();
        }
    }
}

void ngx_nrpc_dummy_read(ngx_event_t* rev)
{
    (void) rev;
    return;
}

void ngx_nrpc_send_response(ngx_event_t* wev)
{
    RpcSession* rpc_session = (RpcSession*)((ngx_connection_t*)wev->data)->data;

    // send all to tcp buf
    rpc_session->set_state(RPC_SESSION_LOGING);
    // return rpc_session->get_log_handler()(rpc_session);
    return rpc_session->finalize();
}

}

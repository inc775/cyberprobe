
#include <cybermon/thread.h>
#include <cybermon/context.h>
#include <cybermon/engine.h>
#include <cybermon/ip.h>

using namespace cybermon;

context_ptr engine::get_root_context(const std::string& liid,
				     const std::string& network)
{
    lock.lock();

    context_ptr c;

    if (contexts.find(liid) == contexts.end()) {
	c = context_ptr(new root_context(*this));
	
	root_context* rp = dynamic_cast<root_context*>(c.get());
	rp->set_liid(liid);
	rp->set_network(network);
	contexts[liid] = c;
    } else
	c = contexts[liid];

    lock.unlock();

    return c;
}

void engine::close_root_context(const std::string& liid)
{
    lock.lock();
    contexts.erase(liid);
    lock.unlock();
}

void engine::process(context_ptr c, const pdu_slice& sl)
{
    ip::process(*this, c, sl);
}

void engine::describe_src(context_ptr p, std::ostream& out)
{

    std::list<context_ptr> l;
    get_context_stack(p, l);

    bool need_sep = false;

    for(std::list<context_ptr>::iterator it = l.begin();
	it != l.end();
	it++) {

	if (need_sep)
	    out << "/";

	(*it)->addr.src.describe(out);

	need_sep = true;
    }

}

void engine::describe_dest(context_ptr p, std::ostream& out)
{

    std::list<context_ptr> l;
    get_context_stack(p, l);

    bool need_sep = false;

    for(std::list<context_ptr>::iterator it = l.begin();
	it != l.end();
	it++) {

	if (need_sep)
	    out << "/";

	(*it)->addr.dest.describe(out);
	need_sep = true;
    }

}

void engine::get_root_info(context_ptr p, std::string& liid, address& a)
{

    while (p) {
	if (p->get_type() == "root") {

	    // Cast to root.
	    root_context& rc = 
		dynamic_cast<root_context&>(*p);
	    
	    liid = rc.get_liid();
	    a = rc.get_trigger_address();

	}
	p = p->parent.lock();
    }

}


void engine::get_network_info(context_ptr p,
			      std::string& net, address& src, address& dest)
{

    src = address();
    dest = address();
    while (p) {
        if (p->get_type() == "root") {
	    root_context& rc = dynamic_cast<root_context&>(*p);
	    net = rc.get_network();
	}
	if (p->get_type() == "ip4") {
	    src = p->addr.src;
	    dest = p->addr.dest;
	}
	if (p->get_type() == "ip6") {
	    src = p->addr.src;
	    dest = p->addr.dest;
	}
	p = p->parent.lock();
    }

}



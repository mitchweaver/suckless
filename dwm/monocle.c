void monocle(Monitor *m) {
	int n = 0;
	Client *c;
	for (c = m->clients; c; c = c->next)
		if (ISVISIBLE(c)) n++;

    for (c = nexttiled(m->clients); c; c = nexttiled(c->next))
		resize(c, m->wx, m->wy, m->ww - 2 * c->bw, m->wh - 2 * c->bw, 0);
}

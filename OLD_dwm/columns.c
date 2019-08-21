void columns(Monitor *m) {
	int i, n, h, w, x, y,mw;
	Client *c;
	for(n = 0, c = nexttiled(m->clients); c; c = nexttiled(c->next), n++);
	if(n == 0) return;
        if(n > m->nmaster) mw = m->nmaster ? m->ww * m->mfact : 0;
        else mw = m->ww;
	for(i = x = y = 0, c = nexttiled(m->clients); c; c = nexttiled(c->next), i++) {
		if(i < m->nmaster) {
			 w = (mw - x) / (MIN(n, m->nmaster)-i);
                         resize(c, x + m->wx, m->wy, w - (2*c->bw), m->wh - (2*c->bw), False);
			x += WIDTH(c);
		}
		else {
			h = (m->wh - y) / (n - i);
			resize(c, x + m->wx, m->wy + y, m->ww - x  - (2*c->bw), h - (2*c->bw), False);
			y += HEIGHT(c);
		}
	}
}

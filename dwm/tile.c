void tile(Monitor *m) {
	unsigned int i, n, h, smh, mw, my, ty;
	Client *c;

	for (n = 0, c = nexttiled(m->clients); c; c = nexttiled(c->next), n++);
	if (n == 0) return;

	if (n > m->nmaster) mw = m->nmaster ? m->ww * m->mfact : 0;
	else mw = m->ww;
	for (i = my = ty = 0, c = nexttiled(m->clients); c; c = nexttiled(c->next), i++)
		if (i < m->nmaster) {
			h = (m->wh - my) / (MIN(n, m->nmaster) - i);
			resize(c, m->wx, m->wy + my, mw - (2*c->bw), h - (2*c->bw), 0);
			my += HEIGHT(c);
		} else {

           smh = m->mh * m->smfact;
           if(!(nexttiled(c->next))) h = (m->wh - ty) / (n - i);
           else h = (m->wh - smh - ty) / (n - i);
           if(h < 10) {
               c->isfloating = True;
               XRaiseWindow(dpy, c->win);
               resize(c, m->mx + (m->mw / 2 - WIDTH(c) / 2), m->my + (m->mh / 2 - HEIGHT(c) / 2), m->ww - mw - (2*c->bw), h - (2*c->bw), False);
               ty -= HEIGHT(c);
           }
           else resize(c, m->wx + mw, m->wy + ty, m->ww - mw - (2*c->bw), h - (2*c->bw), False);
           if(!(nexttiled(c->next))) ty += HEIGHT(c) + smh;
           else ty += HEIGHT(c);
        }
}

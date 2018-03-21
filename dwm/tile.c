void tile(Monitor *m) {
	unsigned int i, n, h, smh, mw, my, ty, scnt;
	Client *c;

	for (n = 0, c = nexttiled(m->clients); c; c = nexttiled(c->next), n++);
	if (n == 0) return;

	if (n > m->nmaster) 
        mw = m->nmaster ? m->ww * m->mfact : 0;
	else 
        mw = m->ww;

	for (i = my = ty = scnt = 0, c = nexttiled(m->clients); c; c = nexttiled(c->next), i++)
		if (i < m->nmaster) {
			h = (m->wh - my) / (MIN(n, m->nmaster) - i);
			if(n == 1)
				resize(c, m->wx, m->wy, m->ww, m->wh, False);
			else
				resize(c, m->wx - c->bw/2, m->wy + my, mw - c->bw/2, h - c->bw, False);
			my += HEIGHT(c) - c->bw;
		} else {
           scnt++;
           smh = m->mh * m->smfact;
           if(!(nexttiled(c->next))) 
               h = (m->wh - ty) / (n - i);
           else 
               h = (m->wh - smh - ty) / (n - i);

           if(h < 10) {
               c->isfloating = True;
               XRaiseWindow(dpy, c->win);
               resize(c, m->wx + mw - c->bw, m->wy + ty, m->ww - mw, h - c->bw, False);
               ty += HEIGHT(c) - c->bw;
           } 

           else if(scnt == 1) resize(c, m->wx + mw, m->wy + ty, m->ww - mw - c->bw*2, h - c->bw/2, False);
                else resize(c, m->wx + mw, m->wy + ty - (c->bw/2*scnt), m->ww - mw - c->bw*2, h + (c->bw/2 * scnt), False);

           if(!(nexttiled(c->next))) 
               ty += HEIGHT(c) + smh;
           else 
               ty += HEIGHT(c);
        }
}

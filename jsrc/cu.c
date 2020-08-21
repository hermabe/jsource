/* Copyright 1990-2007, Jsoftware Inc.  All rights reserved.               */
/* Licensed use only. Any other use is in violation of copyright.          */
/*                                                                         */
/* Conjunctions: Under and Each                                            */

#include "j.h"
#include "ve.h"


static A jteverysp(J jt,A w,A fs){A*wv,x,z,*zv;P*wp,*zp;
 RZ(w);
 AF f1=FAV(fs)->valencefns[0];
 ASSERT(SBOX&AT(w),EVNONCE);
 RZ(z=ca(w));
 wp=PAV(w); x=SPA(wp,x); wv=AAV(x);
 zp=PAV(z); x=SPA(zp,x); zv=AAV(x);
 DQ(AN(x), RZ(*zv++=CALL1(f1,*wv++,fs)););
 R z;
}

#define EVERYI(exp)  {RZ(x=exp); INCORP(x); RZ(*zv++=x); ASSERT(!(SPARSE&AT(x)),EVNONCE);}
     /* note: x can be non-noun */

// u&.>, but w may be a gerund, which makes the result a list of functions masquerading as an aray of boxes
static DF1(jteveryself){R jtevery(jt,w,FAV(self)->fgh[0]);}   // replace u&.> with u and process
A jtevery(J jt, A w, A fs){A * RESTRICT wv,x,z,* RESTRICT zv;
 RZ(w);F1PREFIP;
 if(unlikely(SPARSE&AT(w)))R everysp(w,fs);
 AF f1=FAV(fs)->valencefns[0];   // pointer to function to call
 A virtw; I flags;  // flags are: ACINPLACE=pristine result; JTWILLBEOPENED=nonrecursive result; BOX=input was boxed; ACPERMANENT=input was inplaceable pristine, contents can be inplaced
 flags=ACINPLACE|((I)jtinplace&JTWILLBEOPENED)|(AT(w)&BOX)|((AC(w)>>(ACINPLACEX-ACPERMANENTX))&((I)jtinplace<<(ACPERMANENTX-JTINPLACEWX))&(AFLAG(w)<<(ACPERMANENTX-AFPRISTINEX))&ACPERMANENT);
 // Get input pointer
 I virtblockw[NORMAH];  // space for a virtual block of rank 0
 if(likely(flags&BOX))virtw=*(wv=AAV(w));  // if input is boxed, point to first box
 else{
  // if input is not boxed, use a faux-virtual block to point to the atoms.  Repurpose unneeded wv to hold length
  fauxvirtual(virtw,virtblockw,w,0,ACUC1); AN(virtw)=1; wv=(A*)bpnoun(AT(w));  // note if w has gerunds, it is always boxed & doesn't go through here
 }
 // Allocate result area
 GATV(z,BOX,AN(w),AR(w),AS(w));
 AFLAG(z)=(~flags<<(BOXX-JTWILLBEOPENEDX))&BOX;  // if WILLBEOPENED is NOT set, make the result a recursive box
 I natoms=AN(w); if(!natoms)R z;  // exit if no result atoms
 zv=AAV(z);
 // Get jt flags to pass to next level - only the inplacing flag, if the routine can take it.  We enable inplacing to fs if our input w was inplaceable.  To get actual inplacing we also have to make the contents inplaceable
 jtinplace=(J)((I)jt+((flags>>(ACPERMANENTX-JTINPLACEWX))&(FAV(fs)->flag>>(VJTFLGOK1X-JTINPLACEWX))&JTINPLACEW));
 while(1){
  // If the input was pristine, flag the contents as inplaceable UNLESS they are PERMANENT
  RZ(x=CALL1IP(f1,virtw,fs)); ASSERT(!(SPARSE&AT(x)),EVNONCE); // run the user's verb
  // If z is DIRECT inplaceable, it must be unique and we can inherit them into a pristine result.  Otherwise clear pristinity
  if(AT(x)&DIRECT){
    flags&=SGNIFPRISTINABLE(AC(x))|~ACINPLACE;  // sign bit of flags will hold PRISTINE status of result: 1 if all DIRECT and inplaceable or PERMANENT
  }else{
    // not DIRECT.  result must be non-pristine, and we need to turn off pristinity of x since we are going to incorporate it
    flags&=~ACINPLACE;  // result not pristine
    {I aflg=AFLAG(x); A awbase=x; if(unlikely(aflg&AFVIRTUAL)){awbase=ABACK(x); aflg=AFLAG(awbase);} AFLAG(awbase)=aflg&~AFPRISTINE;}  // x can never be pristine, since is being incorped
  }
  // prepare the result so that it can be incorporated into the overall boxed result
  if(!(flags&JTWILLBEOPENED)) {
   // normal case where we are creating the result box.  Must incorp the result
   realizeifvirtual(x); ra(x);   // Since we are moving the result into a recursive box, we must ra() it.  This plus rifv plus pristine removal=INCORPRA.  We could save some fetches by bundling this code into the RIRECT path
  } else {
   // result will be opened.  It is nonrecursive.  description in result.h.  We don't have to realize or ra
   if(AFLAG(x)&AFUNINCORPABLE){RZ(x=clonevirtual(x));}
   // since we are adding the block to a NONrecursive boxed result,  we DO NOT have to raise the usecount of the block.  And we don't have to mark the usecount non-inplaceable
#if 0  // not clear this is worth doing
   if(ZZFLAGWORD&ZZFLAGCOUNTITEMS){
    // if the result will be razed next, we will count the items and store that in AM.  We will also ensure that the result boxes' contents have the same type
    // and item-shape.  If one does not, we turn off special raze processing.  It is safe to take over the AM field in this case, because we know this is WILLBEOPENED and
    // (1) will never assemble or epilog; (2) will feed directly into a verb that will discard it without doing any usecount modification
#if !ZZSTARTATEND  // going forwards
    A result0=AAV(zz)[0];   // fetch pointer to the first 
#else
    A result0=AAV(zz)[AN(zz)-1];  // fetch pointer to first value stored, which is in the last position
#endif
    I* zs=AS(z); I* ress=AS(result0); I zr=AR(z); I resr=AR(result0); //fetch info
    I diff=TYPESXOR(AT(z),AT(result0))|(MAX(zr,1)^MAX(resr,1)); resr=(zr>resr)?resr:zr;  DO(resr-1, diff|=zs[i+1]^ress[i+1];)  // see if there is a mismatch.  Fixed loop to avoid misprediction
    ZZFLAGWORD^=(diff!=0)<<ZZFLAGCOUNTITEMSX;  // turn off bit if so 
    I nitems=zs[0]; nitems=(zr==0)?1:nitems; AM(zz)+=nitems;  // add new items to count in zz.  zs[0] will never segfault, even if z is empty
   }
   // Note: by checking COUNTITEMS inside WILLBEOPENED we suppress support for COUNTITEMS in \. which sets WILLBEOPENEDNEVER.  It would be safe to
   // count then, because no virtual contents would be allowed.  But we are not sure that the EPILOG is safe, and this path is now off to the side
#endif
  }
  // Store result & advance to next cell
  *zv++=x;
  if(!--natoms)break;  // break to avoid fetching over the end of the input
  if(flags&BOX)virtw=*++wv;else AK(virtw)+=(I)wv;  // advance to next input cell - either by fetching the next box or advancing the virtual pointer to the next atom
 }
 // indicate pristinity of result
 R z;
}

// u&.>, but w may be a gerund, which makes the result a list of functions masquerading as an aray of boxes
static DF2(jtevery2self){R jtevery2(jt,a,w,FAV(self)->fgh[0]);}   // replace u&.> with u and process
A jtevery2(J jt, A a, A w, A fs){A*av,*wv,x,z,*zv;
// todo kludge should rewrite with single flag word
 RZ(a&&w);F2PREFIP;
 AF f2=FAV(fs)->valencefns[1];
 // Get the number of atoms, and the number of times to repeat the short side.
 // The repetition is the count of the surplus frame.
 I rpti;  // number of times short frame must be repeated
 I natoms;  // total # cells
 C flags;  // 20=w is boxed 40=a is boxed 1=w is repeated 2=a is repeated
 {
  I ar=AR(a); I wr=AR(w);
  I cf=ar; A la=w; cf=ar<wr?cf:wr; la=ar<wr?la:a; I lr=ar+wr-cf;  // #common frame, Ablock with long shape, long rank.
  PROD(rpti,lr-cf,AS(la)+cf);
  natoms=MAX(AN(a),AN(w)); natoms=rpti==0?rpti:natoms;  // number of atoms.  Beware of empty arg with surplus frame containing 0; if an arg is empty, so is the result
  flags=(C)(REPSGN(1-rpti)&(SGNTO0(ar-wr)+1));  // if rpti<2, no repeat; otherwise repeat short frame 1 if ar>wr 2 if wr>ar
  // Verify agreement
  ASSERTAGREE(AS(a),AS(w),cf);  // frames must agree
  GATV(z,BOX,natoms,lr,AS(la)); if(!natoms)R z; zv=AAV(z);  // make sure we don't fetch outside empty arg
 }
 A virtw;
 // create virtual blocks if needed
 I virtblockw[NORMAH+1];  // space for a virtual block of rank 0
 if(BOX&AT(w)){flags|=BOX; virtw=*(wv=AAV(w));}  // if input is boxed, point to first box
 else{
  // if input is not boxed, use a faux-virtual block to point to the atoms.  In this case wv is not needed and we use it for the length of an atom
  fauxvirtual(virtw,virtblockw,w,0,ACUC1); AN(virtw)=1; wv=(A*)bpnoun(AT(w));
 }
 A virta;
 I virtblocka[NORMAH+1];  // space for a virtual block of rank 0
 if(BOX&AT(a)){flags|=BOX<<1; virta=*(av=AAV(a));}  // if input is boxed, point to first box
 else{
  // if input is not boxed, use a faux-virtual block to point to the atoms.  In this case av is not needed and we use it for the length of an atom
  fauxvirtual(virta,virtblocka,a,0,ACUC1); AN(virta)=1; av=(A*)bpnoun(AT(a));
 }
 // Loop for each cell.  Increment the pointer unless the side is being repeated and the repeat-count has not expired.
 // Break in the middle of the loop to avoid fetching out of bounds to get the next address from [aw]v
// obsolete  I rpt=rpti; while(1){EVERYI(CALL2(f2,virta,virtw,fs)); if(!--natoms)break; if(!(flags&2)||(--rpt==0&&(rpt=rpti,1))){if(flags&(BOX<<1))virta=*++av;else AK(virta)+=(I)av;} if(!(flags&1)||(--rpt==0&&(rpt=rpti,1))){if(flags&BOX)virtw=*++wv;else AK(virtw)+=(I)wv;} }
 I rpt=rpti=-rpti; while(1){
  EVERYI(CALL2(f2,virta,virtw,fs)); if(!--natoms)break;
  ++rpt; I endrpt=REPSGN(rpt); rpt=rpt==0?rpti:rpt;  // endrpt=0 if end of repeat, otherwise ~0.  Reload rpt at end
  if(!(flags&endrpt&2)){if(flags&(BOX<<1))virta=*++av;else AK(virta)+=(I)av;}  // advance unrepeated arg
  if(!(flags&endrpt&1)){if(flags&BOX)virtw=*++wv;else AK(virtw)+=(I)wv;}
 }
 R z;
}

// apply f2 on items of a or w against the entirety of the other argument.  Pass on rank of f2 to reduce rank nesting
DF2(jteachl){RZ(a&&w&&self); I lcr=AR(a)-1<0?0:AR(a)-1; I lr=lr(self); lr=lcr<lr?lcr:lr; I rr=rr(self); rr=AR(w)<rr?AR(w):rr; R rank2ex(a,w,self,lr,rr,lcr,AR(w),FAV(self)->valencefns[1]);}
DF2(jteachr){RZ(a&&w&&self); I rcr=AR(w)-1<0?0:AR(w)-1; I rr=rr(self); rr=rcr<rr?rcr:rr; I lr=lr(self); lr=AR(a)<lr?AR(a):lr; R rank2ex(a,w,self,lr,rr,AR(a),rcr,FAV(self)->valencefns[1]);}

// u&.v
// PUSH/POP ZOMB is performed in atop/amp/ampco
// under is for when we could not precalculate the inverse.  The verb is in localuse
static DF1(jtunder1){F1PREFIP;DECLFG;A fullf; RZ(fullf=atop(invrecur(fix(sv->localuse.lvp[0],sc(FIXASTOPATINV))),sv->fgh[2])); R (FAV(fullf)->valencefns[0])(FAV(fullf)->flag&VJTFLGOK1?jtinplace:jt,w,fullf);}
static DF2(jtunder2){F2PREFIP;DECLFG;A fullf; RZ(fullf=atop(invrecur(fix(sv->localuse.lvp[0],sc(FIXASTOPATINV))),sv->fgh[2])); R (FAV(fullf)->valencefns[1])(FAV(fullf)->flag&VJTFLGOK2?jtinplace:jt,a,w,fullf);}
// underh has the inverse precalculated, and the inplaceability set from it.  It handles &. and &.: which differ only in rank
static DF1(jtunderh1){F1PREFIP;DECLFGH; R (FAV(hs)->valencefns[0])(jtinplace,w,hs);}
static DF2(jtunderh2){F2PREFIP;DECLFGH; R (FAV(hs)->valencefns[1])(jtinplace,a,w,hs);}
// undco is for when we could not precalculate the inverse
static DF1(jtundco1){F1PREFIP;DECLFG;A fullf; RZ(fullf=atop(inv(sv->localuse.lvp[0]),sv->fgh[2])); R (FAV(fullf)->valencefns[0])(FAV(fullf)->flag&VJTFLGOK1?jtinplace:jt,w,fullf);}
static DF2(jtundco2){F2PREFIP;DECLFG;A fullf; RZ(fullf=atop(inv(sv->localuse.lvp[0]),sv->fgh[2])); R (FAV(fullf)->valencefns[1])(FAV(fullf)->flag&VJTFLGOK2?jtinplace:jt,a,w,fullf);}

// obsolete // u&.> main entry point.  Does not support inplacing.
// obsolete static DF2(jteach2){DECLF; R every2(a,w,fs,f2);}
// versions for rank 0 (including each).  Passes inplaceability through
// if there is only one cell, process it through under[h]1, which understands this type; if more, loop through
static DF1(jtunder10){R jtrank1ex0(jt,w,self,jtunder1);}  // pass inplaceability through
static DF1(jtunderh10){R jtrank1ex0(jt,w,self,jtunderh1);}  // pass inplaceability through
static DF2(jtunder20){R jtrank2ex0(jt,a,w,self,jtunder2);}  // pass inplaceability through
static DF2(jtunderh20){R jtrank2ex0(jt,a,w,self,jtunderh2);}  // pass inplaceability through

static DF1(jtunderai1){DECLF;A x,y,z;B b;I j,n,*u,*v;UC f[256],*wv,*zv;
 RZ(w);
 if(b=LIT&AT(w)&&256<AN(w)){  // long w.  run on all bytecodes, as i. 128 2  and i. 8 32
        df1(x,iota(v2(128L, 2L)),fs); b=x&&256==AN(x)&&NUMERIC&AT(x);
  if(b){df1(y,iota(v2(  8L,32L)),fs); b=y&&256==AN(y)&&NUMERIC&AT(y);}
  if(b){x=vi(x); y=vi(y); b=x&&y;} 
  if(b){u=AV(x); v=AV(y); DO(256, j=*u++; if(j==*v++&&BETWEENO(j,-256,256))f[i]=(UC)(j&255); else{b=0; break;});}  // verify both results the same & in bounds
  if(jt->jerr)RESETERR;
 }         
 if(!b)R from(df1(z,indexof(ds(CALP),w),fs),ds(CALP));
 n=AN(w);
 GATV(z,LIT,n,AR(w),AS(w)); zv=UAV(z); wv=UAV(w);
 if(!bitwisecharamp(f,n,wv,zv))DQ(n, *zv++=f[*wv++];); 
 RETF(z);
}    /* f&.(a.&i.) w */

// u&.v
F2(jtunder){A x,wvb=w;AF f1,f2;B b,b1;C c,uid;I gside=-1;V*u,*v;
 RZ(a&&w);
 if(AT(w)&BOX){
  // Must be the gerund form.  Extract v and remember which argument it will apply to
  ASSERT((AR(w)^1)+(AN(w)^2)==0,EVDOMAIN);  // must be 2-element list
  ASSERT((AN(AAV(w)[0])==0) | (AN(AAV(w)[1])==0),EVDOMAIN);  // one must be empty
  gside=AN(AAV(w)[0])==0;  // the index to the argument v will act on (or -1 if not gerund)
  wvb=fx(AAV(w)[gside]);  // turn the gerund into a verb
 }
 ASSERTVV(a,wvb); v=FAV(wvb);
 c=0; f1=0; f2=0;
 // Set flag with ASGSAFE status of u/v, and inplaceable.  It will stay inplaceable unless we select an uninplaceable processing routine, of we
 // learn that v is uninplaceable.  If v is unknown, keep inplaceable, because we will later evaluate the compound & might be able to inplace then
 I flag = (FAV(a)->flag&v->flag&VASGSAFE) + (VJTFLGOK1|VJTFLGOK2);
 // If v is WILLOPEN, so will the compound be - for all valences
 switch(v->id&gside){  // never special if gerund - this could evaluate to 0 or 1, neither of which is one of these codes
// obsolete  case COPE:  f1=jtunderh10; f2=jtunderh20; flag&=~(VJTFLGOK1|VJTFLGOK2); flag2|=VF2ATOPOPEN1|VF2ATOPOPEN2A|VF2ATOPOPEN2W|VF2BOXATOP1|VF2BOXATOP2; break;   // &.>
 case COPE: R fdef(VF2ATOPOPEN1|VF2ATOPOPEN2A|VF2ATOPOPEN2W|VF2BOXATOP1|VF2BOXATOP2|VF2WILLOPEN1|VF2WILLOPEN2A|VF2WILLOPEN2W,CUNDER,VERB,jteveryself,jtevery2self,a,w,0,flag,0,0,0);   // this is the commonest case.  Return fast, avoiding analysis below
   // The flags are ignored during u&.>, but they can forward through to affect previous verbs.  Also, u&.v"n will be taken over by rank processing
 case CFORK: c=ID(v->fgh[2]); /* fall thru */
 case CAMP:  
  u=FAV(a);  // point to a in a&.w.  w is f1&g1 or (f1 g1 h1)
  if(b1=CSLASH==(uid=u->id)){x=u->fgh[0]; if(AT(x)&VERB){u=FAV(x);uid=u->id;}else uid=0;}   // cases: f&.{f1&g1 or (f1 g1 h1)}  b1=0    f/&.{f1&g1 or (f1 g1 h1)}   b1=1
  b=CBDOT==uid&&(x=u->fgh[1],(((AR(x)-1)&SGNIF(AT(x),INTX))<0)&&BETWEENC(IAV(x)[0],16,32));   // b if f=m b. where m is atomic int 16<=m<=32
  if(CIOTA==ID(v->fgh[1])&&(!c|((c&~1)==CLEFT))&&equ(ds(CALP),v->fgh[0])){   // w is  {a.&i.  or  (a. i. ][)}
   f1=b&b1?jtbitwiseinsertchar:jtunderai1;    // m b./ &. {a.&i.  or  (a. i. ][)}   or  f &. {a.&i.  or  (a. i. ][)}
   f2=((uid==CMAX)|(uid==CMIN))>b1?(AF)jtcharfn2:f2; f2=b>b1?(AF)jtbitwisechar:f2;   // m b. &. {a.&i.  or  (a. i. ][)}   or  >. &. {a.&i.  or  (a. i. ][)}   or f &. {a.&i.  or  (a. i. ][)}
   flag&=~(VJTFLGOK1|VJTFLGOK2);   // not perfect, but ok
  }
  break;
 }
 I flag2=(FAV(wvb)->flag2&(VF2WILLOPEN1|VF2USESITEMCOUNT1))*((VF2WILLOPEN1+VF2WILLOPEN2A+VF2WILLOPEN2W)>>VF2WILLOPEN1X);
 I r=mr(wvb);
 // Create the standard g^:_1 @ (f & g) to use if we have no special processing (not needed if a.&i., but that's rare)
 // if gerund form, use (f g)"lf mg  for a:`v or (f~ g)~"mg rf for v`a:
 // First, create the part after the inverse
 A h; I rmr=r, rlr, rrr; 
 if(gside<0){h=amp(a,wvb); rlr=rrr=rmr;  // normal case, f&g"mg
 }else{
  f1=jtdomainerr1;  // monad not allowed with gerund v
  if(gside==0){rlr=rmr; rrr=(RANKT)FAV(a)->lrr; h=qq(swap(hook(swap(a),wvb)),v2(rlr,rrr));  // (f~ g)~"mw rf
  }else{rlr=FAV(a)->lrr>>RANKTX; rrr=rmr; h=qq(hook(a,wvb),v2(rlr,rrr));  // (f g)"lf mg
  }
 }
 ASSERT(h,EVDOMAIN);
 // If we can calculate the inverse now, do it and so indicate
 if(nameless(wvb)){h=atop(inv(wvb),h); ASSERT(h,EVDOMAIN); flag|=VFUNDERHASINV; } // h must be valid for free.  If no names in w, take the inverse and maek it as done
 // under12 are inplaceable, and pass inplaceability based on the calculated verb.  underh just passes inplaceability through, so we have to transfer the setting from h here,
 // just in case the calculated verb is not inplaceable
 // The standard verbs start with a rank loop; set the flag indicating that
 if(!f1){f1=r?(flag&VFUNDERHASINV?jtunderh1:jtunder1):(flag&VFUNDERHASINV?jtunderh10:jtunder10); flag2|=VF2RANKATOP1; flag&=FAV(h)->flag|(~VJTFLGOK1);}  // allow inplace if v is known inplaceable
 if(!f2){f2=rlr+rrr?(flag&VFUNDERHASINV?jtunderh2:jtunder2):(flag&VFUNDERHASINV?jtunderh20:jtunder20); flag2|=VF2RANKATOP2; flag&=FAV(h)->flag|(~VJTFLGOK2);}  // allow inplace if v is known inplaceable
 RZ(h=fdef(flag2,CUNDER,VERB,(AF)(f1),(AF)(f2),a,w,h,(flag),rmr,rlr,rrr));
 // install wvb into the verb so we can get to it if needed
 FAV(h)->localuse.lvp[0]=wvb;
 R h;
}

F2(jtundco){AF f1=0,f2;I gside=-1, flag=0;
 RZ(a&&w);
 A wvb=w;  // the verb we will take the inverse of
 if(AT(w)&BOX){
  // Must be the gerund form.  Extract v and remember which argument it will apply to
  ASSERT((AR(w)^1)+(AN(w)^2)==0,EVDOMAIN);  // must be 2-element list
  ASSERT((AN(AAV(w)[0])==0) | (AN(AAV(w)[1])==0),EVDOMAIN);  // one must be empty
  gside=AN(AAV(w)[0])==0;  // the index to the argument v will act on (or -1 if not gerund)
  wvb=fx(AAV(w)[gside]);  // turn the gerund into a verb
 }
 ASSERTVV(a,wvb);
 // Set flag with ASGSAFE status of u/v, and inplaceability of f1/f2
 // Create the standard g^:_1 @ (f & g) to use if we have no special processing (not needed if a.&i., but that's rare)
 // if gerund form, use (f g)"lf mg  for a:`v or (f~ g)~"mg rf for v`a:
 // First, create the part after the inverse
 A h;
 if(gside<0){h=ampco(a,wvb); // normal case, f&:g
 }else{
  f1=jtdomainerr1;  // monad not allowed with gerund v
  if(gside==0){h=swap(hook(swap(a),wvb));  // (f~ g)~
  }else{h=hook(a,wvb);  // (f g)
  }
 }
 ASSERT(h,EVDOMAIN);
 // If we can calculate the inverse now, do it and so indicate
 if(nameless(wvb)){h=atop(inv(wvb),h); ASSERT(h,EVDOMAIN); flag|=VFUNDERHASINV; } // h must be valid for free.  If no names in w, take the inverse and maek it as done
 // under12 are inplaceable, and pass inplaceability based on the calculated verb.  underh just passes inplaceability through, so we have to transfer the setting from h here,
 // just in case the calculated verb is not inplaceable
 if(!f1)f1=flag&VFUNDERHASINV?jtunderh1:jtundco1; f2=flag&VFUNDERHASINV?jtunderh2:jtundco2; flag |= (FAV(a)->flag&FAV(wvb)->flag&VASGSAFE) + (FAV(h)->flag&(VJTFLGOK1|VJTFLGOK2));
 RZ(h=fdef(0,CUNDCO,VERB,(AF)(f1),(AF)(f2),a,w,h,flag,RMAX,RMAX,RMAX));
 // install wvb into the verb so we can get to it if needed
 FAV(h)->localuse.lvp[0]=wvb;
 R h;
}

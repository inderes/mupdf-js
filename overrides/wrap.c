#include "emscripten.h"
#include "mupdf/fitz.h"
#include <string.h>

char* makeDto(fz_buffer *buf) {
    char *dto = malloc(4 + buf->len);
    uint32_t buf_len = (uint32_t)buf->len;
    memcpy(dto, &buf_len, 4);
    memcpy(dto + 4, buf->data, buf->len);
    return dto;
}

EMSCRIPTEN_KEEPALIVE
fz_context *createContext(void)
{
	fz_context *ctx = fz_new_context(NULL, NULL, 100<<20);
	fz_register_document_handlers(ctx);
	return ctx;
}

EMSCRIPTEN_KEEPALIVE
void freeContext(fz_context *ctx) {
	fz_drop_context(ctx);
}

EMSCRIPTEN_KEEPALIVE
fz_document *openDocument(fz_context *ctx, const char *filename)
{
    fz_try(ctx) {
		return fz_open_document(ctx, filename);
	}
	fz_catch(ctx) {
	}
	return NULL;
}

EMSCRIPTEN_KEEPALIVE
void freeDocument(fz_context *ctx, fz_document *doc)
{
	fz_drop_document(ctx, doc);
}

EMSCRIPTEN_KEEPALIVE
int countPages(fz_context *ctx, fz_document *doc)
{
	return fz_count_pages(ctx, doc);
}


EMSCRIPTEN_KEEPALIVE
char *drawPageAsHTML(fz_context *ctx, fz_document *doc, int number)
{
	fz_stext_page *text;
	fz_buffer *buf;
	fz_output *out;
	fz_page *lastPage = fz_load_page(ctx, doc, number - 1);

	buf = fz_new_buffer(ctx, 0);
	{
		out = fz_new_output_with_buffer(ctx, buf);
		{
			text = fz_new_stext_page_from_page(ctx, lastPage, NULL);
			fz_print_stext_page_as_html(ctx, out, text, number);
			fz_drop_stext_page(ctx, text);
		}
		fz_write_byte(ctx, out, 0);
		fz_close_output(ctx, out);
		fz_drop_output(ctx, out);
	}
	fz_drop_page(ctx, lastPage);

	char *dto = makeDto(buf);
    fz_drop_buffer(ctx, buf);

	return dto;
}

EMSCRIPTEN_KEEPALIVE
char *drawPageAsSVG(fz_context *ctx, fz_document *doc, int number)
{
	fz_buffer *buf;
	fz_output *out;
	fz_device *dev;
	fz_rect bbox;

	fz_page *lastPage = fz_load_page(ctx, doc, number - 1);

	buf = fz_new_buffer(ctx, 0);
	{
		out = fz_new_output_with_buffer(ctx, buf);
		{
			bbox = fz_bound_page(ctx, lastPage);
			dev = fz_new_svg_device(ctx, out, bbox.x1-bbox.x0, bbox.y1-bbox.y0, FZ_SVG_TEXT_AS_PATH, 0);
			fz_run_page(ctx, lastPage, dev, fz_identity, NULL);
			fz_close_device(ctx, dev);
			fz_drop_device(ctx, dev);
		}
		fz_write_byte(ctx, out, 0);
		fz_close_output(ctx, out);
		fz_drop_output(ctx, out);
	}
	fz_drop_page(ctx, lastPage);

	char *dto = makeDto(buf);
    fz_drop_buffer(ctx, buf);

	return dto;
}

EMSCRIPTEN_KEEPALIVE
char *drawPageAsPNG(fz_context *ctx, fz_document *doc, int number, float dpi)
{
	float zoom = dpi / 72;
	fz_pixmap *pix;
	fz_buffer *buf;
	fz_output *out;


	fz_page *lastPage = fz_load_page(ctx, doc, number - 1);

	buf = fz_new_buffer(ctx, 0);
	{
		out = fz_new_output_with_buffer(ctx, buf);
		{
			pix = fz_new_pixmap_from_page(ctx, lastPage, fz_scale(zoom, zoom), fz_device_rgb(ctx), 0);
			fz_write_pixmap_as_data_uri(ctx, out, pix);
			fz_drop_pixmap(ctx, pix);
		}
		fz_write_byte(ctx, out, 0);
		fz_close_output(ctx, out);
		fz_drop_output(ctx, out);
	}
	fz_drop_page(ctx, lastPage);

	char *dto = makeDto(buf);
    fz_drop_buffer(ctx, buf);

	return dto;
}

EMSCRIPTEN_KEEPALIVE
char *drawPageAsPNGRaw(fz_context *ctx, fz_document *doc, int number, float dpi)
{
	float zoom = dpi / 72;
	fz_pixmap *pix;
	fz_buffer *buf;
	fz_output *out;


	fz_page *lastPage = fz_load_page(ctx, doc, number - 1);


	buf = fz_new_buffer(ctx, 0);
	{
		out = fz_new_output_with_buffer(ctx, buf);
		{
			pix = fz_new_pixmap_from_page(ctx, lastPage, fz_scale(zoom, zoom), fz_device_rgb(ctx), 0);
			fz_write_pixmap_as_png(ctx, out, pix);
			fz_drop_pixmap(ctx, pix);
		}
		fz_write_byte(ctx, out, 0);
		fz_close_output(ctx, out);
		fz_drop_output(ctx, out);
	}
	fz_drop_page(ctx, lastPage);

	char *dto = makeDto(buf);
    fz_drop_buffer(ctx, buf);

	return dto;
}

static fz_irect pageBounds(fz_context *ctx, fz_document *doc, int number, float dpi)
{
	fz_page *lastPage = fz_load_page(ctx, doc, number - 1);
	fz_irect rect = fz_round_rect(fz_transform_rect(fz_bound_page(ctx, lastPage), fz_scale(dpi/72, dpi/72)));
	fz_drop_page(ctx, lastPage);
	return rect;
}


EMSCRIPTEN_KEEPALIVE
int pageWidth(fz_context *ctx, fz_document *doc, int number, float dpi)
{
	fz_irect bbox = pageBounds(ctx, doc, number, dpi);
	return bbox.x1 - bbox.x0;
}

EMSCRIPTEN_KEEPALIVE
int pageHeight(fz_context *ctx, fz_document *doc, int number, float dpi)
{
	fz_irect bbox = pageBounds(ctx, doc, number, dpi);
	return bbox.y1 - bbox.y0;
}

EMSCRIPTEN_KEEPALIVE
char *pageLinks(fz_context *ctx, fz_document *doc, int number, float dpi)
{
	fz_buffer *buf;
	fz_link *links, *link;

	fz_page *lastPage = fz_load_page(ctx, doc, number - 1);

	buf = fz_new_buffer(ctx, 0);
	{
		links = fz_load_links(ctx, lastPage);
		{
			for (link = links; link; link = link->next)
			{
				fz_irect bbox = fz_round_rect(fz_transform_rect(link->rect, fz_scale(dpi/72, dpi/72)));
				fz_append_printf(ctx, buf, "<area shape=\"rect\" coords=\"%d,%d,%d,%d\"",
					bbox.x0, bbox.y0, bbox.x1, bbox.y1);
				if (fz_is_external_link(ctx, link->uri))
					fz_append_printf(ctx, buf, " href=\"%s\">\n", link->uri);
				else
				{
					fz_location linkLoc = fz_resolve_link(ctx, doc, link->uri, NULL, NULL);
					int linkNumber = fz_page_number_from_location(ctx, doc, linkLoc);
					fz_append_printf(ctx, buf, " href=\"#page%d\">\n", linkNumber+1);
				}
			}
		}
		fz_append_byte(ctx, buf, 0);
		fz_drop_link(ctx, links);
	}

	fz_drop_page(ctx, lastPage);

	char *dto = makeDto(buf);

	fz_drop_buffer(ctx, buf);

	return dto;
}

EMSCRIPTEN_KEEPALIVE
char *documentTitle(fz_context *ctx, fz_document *doc)
{
	static char buf[100];
	if (fz_lookup_metadata(ctx, doc, FZ_META_INFO_TITLE, buf, sizeof buf) > 0)
		return buf;
	return "Untitled";
}

EMSCRIPTEN_KEEPALIVE
fz_outline *loadOutline(fz_context *ctx, fz_document *doc)
{
	return fz_load_outline(ctx, doc);
}

EMSCRIPTEN_KEEPALIVE
void freeOutline(fz_context *ctx, fz_outline *outline)
{
	fz_drop_outline(ctx, outline);
}

EMSCRIPTEN_KEEPALIVE
char *outlineTitle(fz_outline *node)
{
	return node->title;
}

EMSCRIPTEN_KEEPALIVE
int outlinePage(fz_context *ctx, fz_document *doc, fz_outline *node)
{
  fz_location next = fz_next_page(ctx, doc, node->page);
  return fz_page_number_from_location(ctx, doc, next);
}

EMSCRIPTEN_KEEPALIVE
fz_outline *outlineDown(fz_outline *node)
{
	return node->down;
}

EMSCRIPTEN_KEEPALIVE
fz_outline *outlineNext(fz_outline *node)
{
	return node->next;
}

EMSCRIPTEN_KEEPALIVE
void dropDto(char *dto) {
	free(dto);
}

EMSCRIPTEN_KEEPALIVE
char *getPageText(fz_context *ctx, fz_document *doc, int number)
{
    fz_page *page;
    fz_stext_page *stext;

    page = fz_load_page(ctx, doc, number - 1);
    stext = fz_new_stext_page_from_page(ctx, page, NULL);
    fz_buffer *buf = fz_new_buffer_from_stext_page(ctx, stext);



    char *dto = makeDto(buf);

    fz_drop_buffer(ctx, buf);

    fz_drop_stext_page(ctx, stext);
    fz_drop_page(ctx, page);


    return dto;
}


EMSCRIPTEN_KEEPALIVE
char *searchPageText(fz_context *ctx, fz_document *doc, int number, char *searchString, int maxHits)
{
    fz_buffer *buf;
    fz_quad hits[maxHits];
    int hit_marks[maxHits];

    if (maxHits <= 0) {
        return NULL;
    }

    int count = fz_search_page_number(ctx, doc, number - 1, searchString, hit_marks, hits, maxHits);

    buf = fz_new_buffer(ctx, 256);

    for (int i = 0; i < count; i++) {
        fz_quad hit = hits[i];
        float x, y, w, h;
        char quadStr[256] = {0};
        x = hit.ul.x;
        y = hit.ul.y;
        w = hit.lr.x - x;
        h = hit.lr.y - y;
        sprintf(quadStr, "%.6f;%.6f;%.6f;%.6f;%u\n", x ,y ,w ,h, hit_marks[i]);
        fz_append_string(ctx, buf, quadStr);
    }

    char *dto = makeDto(buf);

    return dto;
}

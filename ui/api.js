// SPDX-License-Identifier: AGPL-3.0-or-later
// The page talks to the C++ backend through Qt WebChannel.
const backendReady = new Promise((resolve) => {
  new QWebChannel(qt.webChannelTransport, (channel) => resolve(channel.objects.backend));
});

async function invoke(command, payload = {}) {
  const backend = await backendReady;
  const answer = await new Promise((resolve) => backend.call(command, JSON.stringify(payload), resolve));
  const reply = JSON.parse(answer);
  if (reply.error) {
    throw reply.error;
  }
  return reply.state;
}

export const listState = () => invoke("list_state");
export const createTab = () => invoke("create_tab");
export const selectTab = (id) => invoke("select_tab", { id });
export const renameTab = (id, name) => invoke("rename_tab", { id, name });
export const reloadTab = (id) => invoke("reload_tab", { id });
export const openLink = (id, url) => invoke("open_link", { id, url });
export const removeTab = (id) => invoke("remove_tab", { id });

export const configureTab = (id, platformId, name, customUrl) =>
  invoke("configure_tab", { id, platformId, name, customUrl });
